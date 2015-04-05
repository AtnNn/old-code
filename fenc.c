// Written by Etienne Laurin
// Simple xor encryption + useless randomizing

#include <unistd.h>
#include <fcntl.h>
int main(int argc,char **argv){
	int f,n,i=0,s;
	char b,p[32],r[32];
	if(argc!=3){
		write(1,"usage: ./fenc -e|-dfilename\n",28);
		exit(1);
	}
	if((f=open(argv[2],O_RDONLY))==-1){
		write(1,"unable to open file\n",20);
		exit(1);
	}
	if(!(argv[1][1]=='e'||argv[1][1]=='d')){
		write(1,"invalid option\n",15);
		exit(1);
	}
	if((s=read(0,p,32))==-1){
		write(1,"could not read password\n",24);
		exit(1);
	}
	s--;
	p[s]=0;
	if(argv[1][1]=='e'){
		for(;i<s;i++){
			r[i]=rand()%256;
			p[i]^=r[i];
		}
		write(1,p,s);
	}else{
		read(f,r,s);
		for(;i<s;i++){
			r[i]^=p[i];
		}
	}
	while((n=read(f,&b,1))>0){
		b^=r[i%s];
		write(1,&b,1);
		i++;
	}
}
