#define MASTER						0						/* The master PE					*/

#include <bits/stdc++.h>
#include "mpi.h"
#include<omp.h>

using namespace std;

const int MAXN = 16;
const int INF = 1e9;
const int MIN_EDGE_WEIGHT = 1;
const int MAX_EDGE_WEIGHT = 10;

long long fact[MAXN+1];

int random(int l, int r)
{
	return l + rand()%(r-l+1);
}

void precompute_factorial()
{
	fact[0] = 1;

	for(int i=1;i<=MAXN;i++)
	{
		fact[i] = i * fact[i-1];
	}
}

void assign_edge_weights(int** matrix, int N)
{
	for (int i = 0 ; i < N ; i++) {
		for (int j = i+1 ; j < N ; j++) {
			matrix[i][j] = random(MIN_EDGE_WEIGHT,MAX_EDGE_WEIGHT);
			matrix[j][i] = matrix[i][j];
		}
		matrix[i][i] = 0;
	}
}

vector<int> nth_permutation(vector<int>&arr, long long n)
{
	int N = arr.size();

	assert(n<=fact[N]);

	sort(arr.begin(),arr.end());
	
	set<int>st;
	for(int x : arr)st.insert(x);

	vector<int>ans;

	for(int i=0;i<N;i++)
	{
		int cn = 1;
		long long cval = fact[N-1-i];
		while(cval<n)
		{
			cn++;
			cval=(long long)cn*cval;
			cval=(long long)cval/(cn-1);
		}

		long long pval = cval*(cn-1)/cn;
		n -= pval;
		
		auto it = st.begin();
		for(int i=0;i<cn-1;i++)it++;
		ans.push_back(*it);
		st.erase(it);
	}

	return ans;
}

int find_path_cost(int** matrix, vector<int>&arr)
{
	int cost = 0;
	for(int i=1;i<(int)arr.size();i++)
	{
		cost += matrix[arr[i]][arr[i-1]];
	}
	return cost;
}

void print_matrix(int** matrix, int N)
{
	for(int i=0;i<N;i++)
	{
		for(int j=0;j<N;j++)
		{
			cout<<matrix[i][j]<<" ";
		}
		cout<<endl;
	}
}

int main(int argc, char *argv[])
{
	int N = stoi(argv[1]);
	omp_set_num_threads(stoi(argv[2]));

	int			npes;					/* Number of PEs */
	int			mype;					/* My PE number	*/
	int			stat;					/* Error Status	*/
	MPI_Status	status;

	double	start,end;					/* timing */

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &npes );
	MPI_Comm_rank(MPI_COMM_WORLD, &mype );

	precompute_factorial();
	
	int** matrix = new int*[N];
	for (int i = 0; i < N; i++) {
		matrix[i] = new int[N];
	}
	
	// assigning edge weights in MASTER
	if(mype == MASTER){
		assign_edge_weights(matrix, N);
	}
	// sending the edge weights to other PEs
	if(mype == MASTER){
		for(int pe = 1 ; pe < npes ; pe++) {
			for (int i = 0 ; i < N ; i++) {
				MPI_Send(&matrix[i][0], N, MPI_INT, pe, pe, MPI_COMM_WORLD);
			}
		}
	}
	else{
		for (int i = 0 ; i < N ; i++) {
			MPI_Recv( &matrix[i][0], N, MPI_INT, MASTER, mype, MPI_COMM_WORLD, &status);
		}
	}

	MPI_Barrier( MPI_COMM_WORLD );

	// printing the path weight matrix
	// if(mype == MASTER){
	// 	print_matrix(matrix, N);
	// 	cout<<endl;
	// }

	MPI_Barrier( MPI_COMM_WORLD );



	start = MPI_Wtime();							// start time

	// tsp parallel


	int optimal_value = INF;
	int* ans = new int[N+1];

	// divide among PEs

	vector<int> nodes;

	vector<int> my_ans;

	long long nppe = fact[N-1]/npes;
	long long rem = fact[N-1]%npes;

	long long start_perm_ind, end_perm_ind;


	if(rem == 0){
		start_perm_ind = (mype*nppe) + 1;
		end_perm_ind = (mype + 1)*nppe;
	}
	else{
		if(mype < rem){
			start_perm_ind = (mype*(nppe + 1)) + 1;
			end_perm_ind = (mype + 1)*(nppe + 1);
		}
		else{
			start_perm_ind = rem*(nppe + 1) + (mype - rem)*nppe + 1;
			end_perm_ind = rem*(nppe + 1) + (mype + 1 - rem)*nppe;
		}
	}

	// int iter = 0;

	// compute only when some work is assigned to the current PE
	if(start_perm_ind <= end_perm_ind){
		// iter++;

		long long k = end_perm_ind - start_perm_ind + 1;
		
		#pragma omp parallel private(nodes) shared(my_ans,optimal_value)
		{

			for(int i=1;i<N;i++)nodes.push_back(i);
			
			int num = omp_get_num_threads();
			int id = omp_get_thread_num();
			long long iter_per_thread= k/num;
			int extra = k%num;

			// assign iterations to each thread , dividing them equally

			if (id < extra) {
				nodes = nth_permutation(nodes,start_perm_ind - 1 + (id)*(iter_per_thread + 1));
				iter_per_thread=iter_per_thread+1;
			}
			else nodes = nth_permutation(nodes,start_perm_ind - 1 +	(id)*iter_per_thread + extra);
			
			long long i=0;
			do
			{
				vector<int>temp = nodes;
				temp.push_back(0);
				temp.insert(temp.begin(),0);
				int val = find_path_cost(matrix,temp);
				#pragma omp critical
				{
					if(val<optimal_value){	
					optimal_value = val;
					my_ans = temp;
					}
				}
				i++;
				
				next_permutation(nodes.begin(),nodes.end());
			}
			while(i<iter_per_thread);
		}
		copy(my_ans.begin(), my_ans.end(), ans);
	}






	// comparing the optimal values from other PEs in the MASTER
	if(mype == MASTER){

		// receiving ans from all other PEs
		for(int pe = 1 ; pe < npes ; pe++) {

			int tmp_optimal_value;
			int* tmp_ans = new int[N+1];
			MPI_Recv(&tmp_ans[0], (N+1), MPI_INT, pe, pe, MPI_COMM_WORLD, &status);
			MPI_Recv(&tmp_optimal_value, 1, MPI_INT, pe, pe, MPI_COMM_WORLD, &status);

			if(tmp_optimal_value<optimal_value){
				optimal_value = tmp_optimal_value;
				copy(tmp_ans, tmp_ans+N+1, ans);
			}
		}
	}
	else{
		// send ans to MASTER
		MPI_Send(&ans[0], (N+1), MPI_INT, MASTER, mype, MPI_COMM_WORLD);
		MPI_Send(&optimal_value, 1, MPI_INT, MASTER, mype, MPI_COMM_WORLD);
	}

	MPI_Barrier( MPI_COMM_WORLD );

	end = MPI_Wtime();								// end time
	
	// printing the minimum cost path
	// if(mype == MASTER){
	// 	for (int i = 0; i < N+1; i++) {
	// 		cout << ans[i] << ' ';
	// 	}
	// 	cout<<endl<<endl;
	// }

	// printing the minimum path cost
	// if(mype == MASTER){
	// 	int cost = 0;
	// 	for(int i=1;i<N+1;i++)
	// 	{
	// 		cost += matrix[ans[i]][ans[i-1]];
	// 	}
	// 	cout<<cost<<endl<<endl;
	// }

	// printing the run-time
	if(mype == MASTER){
		cout << fixed << setprecision(5) << (end-start) << endl;
	}

	// debugging
	// for (int pe = 0 ; pe < npes ; pe++){
	// 	if (mype == pe){
	// 		cout<<"PE "<<pe<<" :"<<endl;
	// 		// for(int it = 0 ; it < N+1 ; it++) cout<<ans[it]<<" ";
	// 		cout<<"Iterations - "<<iter<<endl;
	// 	}
	// 	MPI_Barrier( MPI_COMM_WORLD );
	// }

	// deleting allocated memory
	delete ans;

	MPI_Finalize();

}