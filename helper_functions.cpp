#include<bits/stdc++.h>

using namespace std;

void swap(int &a, int &b)
{
	int temp = a;
	a = b;
	b = temp;
}

long long fact(int n)
{
	long long ans = 1;
	for(int i=1;i<=n;i++)
	{
		ans *= i;
	}
	return ans;
}

bool nxt_permutation(int *arr, int n)
{
	bool nxt_permutation_possible = false;

	int fi = -1;
	
	for(int i=n-2;i>=0;i--)
	{
		if(arr[i+1] > arr[i])
		{
			nxt_permutation_possible = true;
			fi = i;
			break;
		}
	}

	if(!nxt_permutation_possible)return false;

	int next_greater_ele = arr[fi+1], next_greater_ele_ind = fi+1;

	for(int i=fi+2;i<n;i++)
	{
		if(arr[i] > arr[fi] && arr[i] < next_greater_ele)
		{
			next_greater_ele = arr[i];
			next_greater_ele_ind = i;
		}
	}

	swap(arr[fi],arr[next_greater_ele_ind]);

	//Reverse
	int li = fi+1, ri = n-1;
	while(li < ri)
	{
		swap(arr[li],arr[ri]);
		li++;
		ri--;
	}

	return true;
}

//Input array should be sorted
bool nth_permutation(int *arr, int arrsize, int n)
{
	if(n>fact(arrsize))return false;

	bool taken[arrsize];

	for(int i=0;i<arrsize;i++)taken[i] = false;
	
	int *ans = new int[arrsize];

	for(int i=0;i<arrsize;i++)
	{
		int cn = 1;
		long long cval = fact(arrsize-1-i);

		while(cval<n)
		{
			cn++;
			cval=(long long)cn*cval;
			cval=(long long)cval/(cn-1);
		}

		long long pval = cval*(cn-1)/cn;
		n -= pval;

		for(int j=0;j<arrsize;j++)
		{
			if(!taken[j])
			{
				cn--;
				if(cn==0)
				{
					ans[i] = arr[j];
					taken[j] = true;
					break;
				}
			}
		}
	}

	for(int i=0;i<arrsize;i++)
	{
		arr[i] = ans[i];
	}
	free(ans);
	return true;
}


////////////Testing///////////////////////////////////

bool check(int *a, int *b, int n)
{
	for(int i=0;i<n;i++)
	{
		if(a[i]!=b[i])return false;
	}
	return true;
}
void printarr(int *a, int n)
{
	for(int i=0;i<n;i++)cout<<a[i]<<" ";
	cout<<endl;
}
void test(int n)
{
	int *a = new int[n];
	int *b = new int[n];
	int *c = new int[n];

	for(int i=0;i<n;i++)
	{
		a[i] = i+1;
		b[i] = i+1;
		//c[i] = i+1;
	}

	int i = 0;
	do
	{
		
		for(int j=0;j<n;j++)
		{
			c[j] = j+1;
		}
		
		assert(check(a,b,n));
		i++;
		nth_permutation(c,n,i);
		// printarr(a,n);
		// printarr(b,n);
		// printarr(c,n);
		assert(check(c,b,n));

		cout<<i<<endl;
		nxt_permutation(a,n);

	}while(next_permutation(b,b+n));
}
//////////////////////////////////////////////////////
int main()
{
	test(9);
}