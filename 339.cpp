#include<bits/stdc++.h>
using namespace std;
int main(){
	int y,s,i;
	cin>>y>>s>>i;
	if(y<80){
		y=80;
	}
	if(s<80){
		s=80;	
	}
	if(i<80){
		i=80;
	}
	if(y+s+i>270){
		cout<<"Yes"<<endl;
	}else{
		cout<<"No"<<endl;
	}
	return 0;
	
}
