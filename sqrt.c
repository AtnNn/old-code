#include <stdio.h>
int main(int argc,char**argv){
	int n,i=1,r=0;
	if(argc!=2){
		printf("%s <number> - find square root of number\n",argv[0]);
		return -1;
	}
	for(n=atoi(argv[1]);n>=0;r++){
		n-=i;
		i+=2;
	}
	printf("%s %d",n+i-2==0?"exactly":"about",r-1);
	return 0;
}