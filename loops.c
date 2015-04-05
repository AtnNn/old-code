// Written by Etienne Laurin

// nested loops are boring...

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void g(int *a, int n){
  printf("%d %d %d\n",a[0],a[1],a[2]);
}

void loop(int x, int y, void(*func)(int*,int)){
  int n;
  int *o = malloc((x+1) * sizeof(int));
  memset(o, 0, (x+1) * sizeof(int));
  while(!o[x]){
    g(o,x);
    for(n=0;o[n]==y-1;n++)
      o[n]=0;
    o[n]++;
  }
}

int main(){
  int i[3];
  loop(3, 4, g);
  puts("---");
  for(i[2]=0;i[2]<4;i[2]++)
    for(i[1]=0;i[1]<4;i[1]++)
      for(i[0]=0;i[0]<4;i[0]++)
	g(i, 3);
  return 0;
}
