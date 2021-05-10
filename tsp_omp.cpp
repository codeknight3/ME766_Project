#include<bits/stdc++.h>
#include <ctime>
#include <chrono>
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

void assign_edge_weights(vector<vector<int>>&matrix)
{
	int n = matrix.size();

	for(int i=0;i<n;i++)
	{
		for(int j=i+1;j<n;j++)
		{
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

int find_path_cost(vector<vector<int>>&matrix, vector<int>&arr)
{
	int cost = 0;
	
	for(int i=1;i<(int)arr.size();i++)
	{
		cost += matrix[arr[i]][arr[i-1]];
	}
	return cost;
}

vector<int> tsp_omp(vector<vector<int>>&matrix)
{
	int n = matrix.size();

	int optimal_value = INF;
	
	vector<int>ans;
	
	vector<int>nodes;

	long int k=fact[n-1];
	
	#pragma omp parallel private(nodes) shared(ans,optimal_value)
	{

	  for(int i=1;i<n;i++)nodes.push_back(i);
	  
	  int num = omp_get_num_threads();
	  int id = omp_get_thread_num();
	  long int iter_per_thread= k/num;
	  int extra = k%num;
	  
	  if (id<extra) {
	    nodes = nth_permutation(nodes,(id)*(iter_per_thread+1));
	    iter_per_thread=iter_per_thread+1;
	  }
	  else nodes = nth_permutation(nodes,(id)*iter_per_thread+extra);
	  
	  int i=0;
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
	  		ans = temp;
	  	  }
	  	}
	  	i++;
	  	
	  	next_permutation(nodes.begin(),nodes.end());
	  }
	  while(i<iter_per_thread);
	}
	
	return ans;	
}

void print_matrix(vector<vector<int>>& matrix)
{
	int N = matrix.size();

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

	int threads = stoi(argv[2]);

	omp_set_num_threads(threads);

	precompute_factorial();

	vector<vector<int>>matrix(N,vector<int>(N,0));
	assign_edge_weights(matrix);


	auto start = std::chrono::high_resolution_clock::now();		// start time

	vector<int>ans = tsp_omp(matrix);

	auto finish = std::chrono::high_resolution_clock::now();	        // end time


    // printing the minimum path cost
	//cout<<find_path_cost(matrix,ans)<<endl;
	//cout<<endl;

	// printing the run-time
	chrono::duration<double> elapsed = finish - start;
	cout << fixed << setprecision(5) <<elapsed.count() << endl;
}
