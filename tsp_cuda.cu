#include<bits/stdc++.h>

using namespace std;

const int THREADS_PER_BLOCK = 1024;
const int BLOCKS = 50;

const int MAXN = 16;
const int INF = 1e9;
const int MIN_EDGE_WEIGHT = 1;
const int MAX_EDGE_WEIGHT = 10;


long long factorial[MAXN+1];

__managed__ int block_optimal_values[BLOCKS];
__managed__ int block_optimal_paths[BLOCKS][MAXN+1];

/////////////////// Host Functions ///////////////////

__host__ int random(int l, int r) {
  return l + rand()%(r-l+1);
}

__host__ void precompute_factorial() {
	factorial[0] = 1;

	for(int i=1;i<=MAXN;i++)
	{
		factorial[i] = i * factorial[i-1];
	}
}

__host__ void assign_edge_weights(int* matrix, int N) {
	for (int i = 0 ; i < N ; i++) {
		for (int j = i+1 ; j < N ; j++) {
			matrix[i*N + j] = random(MIN_EDGE_WEIGHT,MAX_EDGE_WEIGHT);
			matrix[j*N + i] = matrix[i*N + j];
		}
		matrix[i*N + i] = 0;
	}
}


__host__ void print_matrix(int* matrix, int N) {
	for(int i=0; i<N; i++) {
		for(int j=0; j<N; j++) {
			cout << matrix[i*N + j] << " ";
		}
		printf("\n");
	}
}

/////////////////// Device Functions ///////////////////

__device__ void swap(int &a, int &b) {
	int temp = a;
	a = b;
	b = temp;
}

__device__ long long fact(int n) {
	long long ans = 1;
	for(int i=1;i<=n;i++) {
		ans *= i;
	}
	return ans;
}

__device__ bool nxt_permutation(int *arr, int n) {
	bool nxt_permutation_possible = false;

	int fi = -1;
	
	for(int i=n-2;i>=0;i--) {
		if(arr[i+1] > arr[i]) {
			nxt_permutation_possible = true;
			fi = i;
			break;
		}
	}

	if(!nxt_permutation_possible)return false;

	int next_greater_ele = arr[fi+1], next_greater_ele_ind = fi+1;

	for(int i=fi+2;i<n;i++) {
		if(arr[i] > arr[fi] && arr[i] < next_greater_ele) {
			next_greater_ele = arr[i];
			next_greater_ele_ind = i;
		}
	}

	swap(arr[fi],arr[next_greater_ele_ind]);

	//Reverse
	int li = fi+1, ri = n-1;
	while(li < ri) {
		swap(arr[li],arr[ri]);
		li++;
		ri--;
	}

	return true;
}

//Input array should be sorted
__device__ bool nth_permutation(int *arr, int arrsize, long long n) {
	if(n>fact(arrsize))return false;

    // Assuming arrSize = N+1
	bool taken[MAXN];

	for(int i=0; i<arrsize; i++) taken[i] = false;
	
	int *ans = new int[arrsize];

	for(int i=0; i<arrsize; i++) {
		int cn = 1;
		long long cval = fact(arrsize-1-i);

		while(cval<n) {
			cn++;
			cval=(long long)cn*cval;
			cval=(long long)cval/(cn-1);
		}

		long long pval = cval*(cn-1)/cn;
		n -= pval;

		for(int j=0; j<arrsize; j++) {
			if(!taken[j]) {
				cn--;
				if(cn==0) {
					ans[i] = arr[j];
					taken[j] = true;
					break;
				}
			}
		}
	}

	for(int i=0; i<arrsize; i++) {
		arr[i] = ans[i];
	}
	free(ans);
	return true;
}

__device__ int find_path_cost(int* matrix, int* arr, int arrsize, int n) {   
	int cost = 0;
	for(int i=1; i<arrsize; i++) {
        int to = arr[i];
        int from = arr[i-1];
		cost += matrix[to*n + from];
	}
	return cost;
}

/////////////////// Global Functions ///////////////////

__global__ void tsp_cuda(int* matrix, int* path, long long* factorials, int N) {

	__shared__ int thread_optimal_values[THREADS_PER_BLOCK];
    __shared__ int* thread_optimal_paths[THREADS_PER_BLOCK];

    int thread = threadIdx.x + blockIdx.x * blockDim.x;

    thread_optimal_values[threadIdx.x] = INF;
    thread_optimal_paths[threadIdx.x] = new int[N+1];

    long long iter_per_thread = factorials[N-1] / (BLOCKS * THREADS_PER_BLOCK);

    int arr[MAXN-1];
    for (int i = 1; i < N; i++) arr[i-1] = path[i];
    nth_permutation(arr, N-1, (thread * iter_per_thread) + 1);

	// Last thread of all handles the permutations not entirely divisible by the total threads in all blocks
	if (thread == (BLOCKS * THREADS_PER_BLOCK) - 1) {
		iter_per_thread += factorials[N-1] % (BLOCKS * THREADS_PER_BLOCK);
	}

    long long iter = 0;
    do {
        
        int temp_path[MAXN+1];
        temp_path[0] = 0;
        for (int i = 1; i < N; i++) temp_path[i] = arr[i-1];
        temp_path[N] = 0;


        int val = find_path_cost(matrix, temp_path, N+1, N);

        if(val < thread_optimal_values[threadIdx.x])
		{
			thread_optimal_values[threadIdx.x] = val;
            for (int i = 0; i < N+1; i++) thread_optimal_paths[threadIdx.x][i] = temp_path[i];
		}

        iter++;
        nxt_permutation(arr, N-1);
        
    } while (iter < iter_per_thread);

    __syncthreads();

	if (threadIdx.x == 0) {
        int optimal_cost = INF;
        for (int i = 0; i < THREADS_PER_BLOCK; i++) {
            if (thread_optimal_values[i] < optimal_cost) {
                optimal_cost = thread_optimal_values[i];
				block_optimal_values[blockIdx.x] = thread_optimal_values[i];
                for (int j = 0; j < N+1; j++) {
                    block_optimal_paths[blockIdx.x][j] = thread_optimal_paths[i][j];
                }
            }           
        }
    }
}


//////////////////////////////////////////////////////////////

int main(int argc, char **argv) {

    const int N = stoi(argv[1]);

    precompute_factorial();


    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    int* matrix = new int[N*N];


    int path[N+1];
    path[0] = 0;
    path[N] = 0;
    for (int i = 1; i < N; i++) path[i] = i;

    assign_edge_weights(matrix, N);

    print_matrix(matrix, N);
	
	for (int i = 0; i < BLOCKS; i++){
		block_optimal_values[i] = INF;
	}

    int *dev_matrix, *dev_path;
    long long *dev_factorial;
    int mat_size = N*N*sizeof(int);
    int path_size = (N+1)*sizeof(int);
    int factorial_size = (MAXN+1)*sizeof(long long);

    cudaMalloc((void **)&dev_matrix, mat_size);
    cudaMalloc((void **)&dev_path, path_size);
    cudaMalloc((void **)&dev_factorial, factorial_size);

    cudaEventRecord(start);

    // Copy inputs from host to device
    cudaMemcpy(dev_matrix, matrix, mat_size, cudaMemcpyHostToDevice);
    cudaMemcpy(dev_path, path, path_size, cudaMemcpyHostToDevice);
    cudaMemcpy(dev_factorial, factorial, factorial_size, cudaMemcpyHostToDevice);

    // Launch the TSP kernel
	tsp_cuda<<<BLOCKS, THREADS_PER_BLOCK>>>(dev_matrix, dev_path, dev_factorial, N);

	cudaDeviceSynchronize();
	cudaDeviceSynchronize();

	int optimal_cost = INF;
	for (int i = 0; i < BLOCKS; i++) {
		if (block_optimal_values[i] < optimal_cost) {
			optimal_cost = block_optimal_values[i];
			for (int j = 0; j < N+1; j++) {
				path[j] = block_optimal_paths[i][j];
			}
		}	
	}

    cudaEventRecord(stop);

    cudaEventSynchronize(stop);
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);

    // printing the minimum cost path
    printf("Minimum Cost Path: ");
    for (int i = 0; i < N+1; i++) {
        printf("%d ", path[i]);
    }
    printf("\n");

    // printing the minimum cost path
    int cost = 0;
    for(int i=1; i<N+1; i++) {
        cost += matrix[path[i]*N + path[i-1]];
    }
    printf("Path cost: %d \n", cost);

    // printing the run-time
    printf("Time taken: %f s\n", milliseconds*0.001);

    cudaFree(dev_matrix);
    cudaFree(dev_path);
	cudaFree(dev_factorial);
}

