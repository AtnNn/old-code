// Written by Etienne Laurin

// runs perl code from inside <@ @> tags

#include <EXTERN.h>
#include <perl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#define TAG '@'
static PerlInterpreter *my_perl;
int main(int argc, char **argv){
	struct stat sb;
	char *buf,*last;
	FILE *fp;
	fp=fopen(argv[argc-1],"r");
	if(!fp)return fprintf(stderr,"unable to open file `%s'\n",argv[argc-1]);
	strcpy(argv[argc-1],"-e0");//filename should be long enough :P
	fstat(fileno(fp),&sb);
	last=buf=malloc(sb.st_size+1);
	if(!buf)return fprintf(stderr,"unable to malloc %d bytes\n",sb.st_size+1);
	fread(buf,1,sb.st_size,fp);
	buf[sb.st_size]=0;
	my_perl = perl_alloc();
	perl_construct(my_perl);
	perl_parse(my_perl, NULL, argc, argv, NULL);
	perl_run(my_perl);
	for(;*buf;buf++){
		if(*buf=='<'&&buf[1]==TAG){
			*buf=0;
			fputs(last,stdout);
			last=buf+=2;
			for(;*buf;buf++){
				if(*buf==TAG&&buf[1]=='>'){
					*buf=0;
					eval_pv(last,TRUE);
					last=buf+=2;
					break;
				}
			}
			if(!buf){
				eval_pv(last,TRUE);
				break;
			}
		}
	}
	fputs(last,stdout);
	perl_destruct(my_perl);
	perl_free(my_perl);
}
