#define MASTER						0						/* The master PE					*/

#include <bits/stdc++.h>
#include "mpi.h"

using namespace std;

const int MAXN = 12;
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
	int N = 11;

	int			npes;					/* Number of PEs */
	int			mype;					/* My PE number  */
	int			stat;					/* Error Status  */
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
	if(mype == MASTER){
		print_matrix(matrix, N);
		cout<<endl;
	}

	MPI_Barrier( MPI_COMM_WORLD );

	start = MPI_Wtime();							// start time

	// tsp parallel

	int optimal_value = INF;
	int* ans = new int[N+1];

	// divide among PEs

	vector<int> my_ans;

	long long nppe = fact[N-1]/npes;

	vector<int>nodes;
	for(int i=1;i<N;i++)nodes.push_back(i);
	vector<int>nodes_begin = nth_permutation(nodes, ((mype*nppe) + 1));
	vector<int>nodes_end = nth_permutation(nodes, (mype + 1)*nppe);

	// int iter = 0;
	do
	{
		vector<int>temp = nodes_begin;
		temp.push_back(0);
		temp.insert(temp.begin(),0);
		int val = find_path_cost(matrix, temp);
		if(val < optimal_value)
		{
			optimal_value = val;
			my_ans = temp;
		}

		if(nodes_begin == nodes_end) break;
		// iter++;

	}while(next_permutation(nodes_begin.begin(),nodes_begin.end()));
	copy(my_ans.begin(), my_ans.end(), ans);

	// comparing the optimal values from other PEs in the MASTER
	if(mype == MASTER){

		// creating arrays to store values received from other PEs

		int* tmp_optimal_value = new int[npes];
		int** tmp_ans = new int*[npes];
		for(int pe = 0 ; pe < npes ; pe++){
			tmp_ans[pe] = new int[N+1];
		}
		tmp_optimal_value[mype] = optimal_value;
		tmp_ans[mype] = ans;

		// receiving ans from all other PEs
		for(int pe = 1 ; pe < npes ; pe++) {
			MPI_Recv(&tmp_ans[pe][0], (N+1), MPI_INT, pe, pe, MPI_COMM_WORLD, &status);
			MPI_Recv(&tmp_optimal_value[pe], 1, MPI_INT, pe, pe, MPI_COMM_WORLD, &status);
		}

		// finding index of minimum optimal value
		int minElementIndex = min_element(tmp_optimal_value,tmp_optimal_value+npes) - tmp_optimal_value;

		delete ans;

		// copying optimal ans to ans
		copy(tmp_ans[minElementIndex], tmp_ans[minElementIndex]+N+1, ans);
		
		// sending ans to other PEs
		for(int pe = 1 ; pe < npes ; pe++) {
			MPI_Send(&ans[0], (N+1), MPI_INT, pe, pe, MPI_COMM_WORLD);
		}

		// deleting allocated memory
		for(int pe = 1 ; pe < npes ; pe++){
			delete tmp_ans[pe];
		}
		delete tmp_optimal_value;
		delete tmp_ans;
	}
	else{
		// send ans to MASTER
		MPI_Send(&ans[0], (N+1), MPI_INT, MASTER, mype, MPI_COMM_WORLD);
		MPI_Send(&optimal_value, 1, MPI_INT, MASTER, mype, MPI_COMM_WORLD);

		// receiving optimal ans from MASTER
		MPI_Recv(&ans[0], (N+1), MPI_INT, MASTER, mype, MPI_COMM_WORLD, &status);
	}

	MPI_Barrier( MPI_COMM_WORLD );

	end = MPI_Wtime();								// end time
	
	// printing the minimum cost path
    if(mype == MASTER){
		for (int i = 0; i < N+1; i++) {
	        cout << ans[i] << ' ';
	    }
	    cout<<endl<<endl;
	}

    // printing the minimum path cost
    if(mype == MASTER){
		int cost = 0;
		for(int i=1;i<N+1;i++)
		{
			cost += matrix[ans[i]][ans[i-1]];
		}
		cout<<cost<<endl<<endl;
	}

	// printing the run-time
    if(mype == MASTER){
		cout<<"Run-Time : "<< (end-start) << endl;
	}

	MPI_Finalize();

}