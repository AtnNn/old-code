/* savemodes.c by AtnNn */ 
#include <sys/types.h>
#include <sys/stat.h>
int main(int argc, char **argv){int i;struct stat sb;for(i=1;i<argc;i++){stat(argv[i],&sb);printf("chmod %o %s\n",sb.st_mode%010000,argv[i]);}}
