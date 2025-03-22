#include<bits/stdc++.h>
using namespace std;
int main(){
	int a,b,i;
	cin>>a>>b;
	i=a;
	while(i<=b){
		if(i%7==0||i%5==0){
			cout<<i<<endl;
		}
		i++;
	}
	return 0;
}
