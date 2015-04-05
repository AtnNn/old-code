// Written by Etienne Laurin

// My sinuses!

#include <math.h>
#include <stdio.h>

#define NUMOF(I) (("ydCC@"[I/2]>>(4*!(I%2)))&15)

int mysin(int angle){
  int i,ret=0,t,a;
  a=angle/2;
  for(i=0;i<9;i++){
    t=NUMOF(i);
    if(a<t){
      t=a;
      i+=9;
    }
    ret+=t*(9-i%9);
    a-=t;
  }
  if(angle%2)
    ret+=(9-i%9)>>1;
  return ret;
}
int m(int b){
	 int i,r=0,t,a;
	  a=b>>1;
	   for(i=0;i<9;i++){
		     t=("ydCC@"[i>>1]>>(4*!(i%2)))&15;
		       r+=(a<t?(i+=9,t=a):t)*(9-i%9);
		         a-=t;
			  }
	    return r+((b%2)?(9-i%9)>>1:0);
}
int main(int argc, char **argv){
  int n;
  n=atoi(argv[1]);
  printf("mine: %d\nreal: %d\n",
	 m(n),
	 (int)(sinf(n/180.*3.1415926)*255));
  return 0;
}
