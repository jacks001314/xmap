#include<stdio.h>
int main(){
	int a,b;
	scanf("%d%d",&a,&b);
	if(a>b){
		printf("%d大于%d\n",a,b);
	}
	else if(a==b){
		printf("%d等于%d\n",a,b);
	}else{
		printf("%d小于%d\n",a,b);
	}
	return 0;
}

