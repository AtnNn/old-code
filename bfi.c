/*
 * BFI - yet another brainfuck interpreter by Etiene Laurin
 *
 *       This code is provided as an example.
 *       Brainfuck coders are hereby encouraged to
 *       write their own interpreter and compiler
 *       and to use less global variables.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int memsize = 1000; // size of memory array
unsigned char *mem; // memory array
int p;              // index of memory array
FILE *in;           // file to read
char *codeptr;      // code pointer
char *code;         // the code
int allow = 0;      // allow char underflows/overflows
int read_i=0;       // was static in bfi_read, i lifted it so i could reset it
int eof=0;

char bfi_read(){ // my own buffered getchar()
  static char buf[256]="";
  if(read_i==-1){ // error or EOF
    return 0;
  }else if(buf[read_i]){ // buffer not empty
    return buf[read_i++];
  }else{ // buffer empty
    read_i=fread(buf,1,255,in); // weird behaviour: need to press ^D twice
    if(read_i<=0){
      read_i=-1;
      buf[0]=0;
      return 0;
    }
    buf[read_i]=0;
    read_i=1;
    return buf[0];
  }
}

int run(int y){ // runs the brainfuck pointed to by codeptr
  char *start;  // for loops, initial position of codeptr
  int n;        // int variable
  start=codeptr;
  if(!y&&!mem[p]){
    n=1;        // n used to count depth
    while(n){   // skip to the first unmatched ']'
      codeptr++;// assuming that previous character is a '['
      if(!*codeptr){
	return 1; // unmatched []'s error
      }
      if(*codeptr=='['){
	n++;
      }else if(*codeptr==']'){
	n--;
      }
    }
    codeptr++;
    return 0; // 0 = success
  }
  for(;;){ // run the code
    switch(*(codeptr++)){
    case '.':
      putchar(mem[p]);
      break;
    case ',':
      n=bfi_read();
      mem[p]=n?n:eof;
      break;
    case '<':
      if(--p<0){
	return -3; // negative memory error
      }
      break;
    case '>':
      if(++p>=memsize){
	mem=realloc(mem,memsize*2); // woohoo, infinite memory
	if(mem==NULL){
	  return -4; // system error, will call perror
	}
	memset(mem+memsize,0,memsize);
	memsize*=2;
      }
      break;
    case '+':
      if(mem[p]++==0xFF&&!allow){
	return -1;
      }
      break;
    case '-':
      if(mem[p]--==0&&!allow){
	return -2;
      }
      break;
    case '[':
      n=run(0); // it's that simple
      if(n<0){
	return n;
      }else if(n>0){
	return n++;
      }
      break;
    case ']': // loop back at a matching ']'
      if(y){  // unless this is the main code, then we have an error
	return 1;
      }else if(mem[p]){
	codeptr=start;
      }else{  // or if mem[p] is nil, then we go on
	return 0;
      }
      break;
    case '#': // only when called with -debug, dumps first 20 elements
      fprintf(stderr,"memory: %d  code: %d\n"
	             "%d %d %d %d %d %d %d %d %d %d "
	             "%d %d %d %d %d %d %d %d %d %d\n"
	      ,p,codeptr-code,
	      mem[0],mem[1],mem[2],mem[3],mem[4],
	      mem[5],mem[6],mem[7],mem[8],mem[9],
	      mem[10],mem[11],mem[12],mem[13],mem[14],
	      mem[15],mem[16],mem[17],mem[18],mem[19]);
      break;
    case 0:
      return !y; // y is 1 when called from main, and 0 otherwise
    }
  }
}

int main(int argc, char **argv){
  int codesize = 0; // size of our code
  char b;           // i forget what b is
  int n,i,c,l;      // i is a loop counter...
  int debug = 0;    // debug and seperate are commandline options
  int seperate = 0;
  code = *argv;     // temp use of a char * to store the command name for -help
  for(argv++;*argv&&**argv=='-';argv++){ // cheap arg parsing
    if(!strcmp(*argv,"-debug")||!strcmp(*argv,"-d")){
      debug = 1;
    }else if(!strcmp(*argv,"-s")||!strcmp(*argv,"-seperate")){
      seperate=1;
    }else if(!strcmp(*argv,"-a")||!strcmp(*argv,"-allow")){
      allow=1;
    }else if(!strcmp(*argv,"-e")||!strcmp(*argv,"-eof")&&argv[1]){
      eof=atoi(*++argv);
    }else{
      printf("BFI - yet another brainfuck intepreter by AtnNn\n"
	     "usage: %s [-h|-help] [-d|-debug] [-s|-seperate] [-a|-allow]"
	     " [infile]\n"
	     "\t-help     print this text\n"
	     "\t-debug    break and dump array on #\n"
	     "\t-seperate code and input seperated by !\n"
	     "\t-allow    allow byte overflow and underflow\n"
	     "\t-eof <N>  read N with , on eof or error",code);
      return 0;
    }
  }
  in=stdin; // if !*argv
  if(*argv){
    if(argv[1]){
      fputs("Error parsing command line arguments\n",stderr);
      return 1;
    }
    in=fopen(*argv,"r");
    if(!in){
      perror("fopen");
      return 2;
    }
  }
  codesize=0; // code array is empty
  l=0;        // i still dont know what l is for
  code=NULL;  // well not empty, just NULL
  c=0;        // index into code
  for(b=bfi_read();b;b=bfi_read()){ // read the code
    if(seperate&&b=='!'){
      break;
    }
    if(b=='<'||b=='>'||b=='['||b==']'||
       b=='.'||b==','||b=='+'||b=='-'||
       (debug&&b=='#')){    // take only reconized characters
      if(c>=codesize){
	codesize+=200;
	code=(char*)realloc(code,codesize); // as many as we need
	if(!code){
	  perror("realloc");
	  return 3;
	}
      }
      code[c++]=b; // this could be the only instruction in this loop
    }              // but gladly C has no dynamic arrays
  }
  if(code==NULL){
    return 0; // shortest brainfuck quine:
  }
  if(!seperate){
    if(*argv){
      fclose(in);
      in=stdin;
    }
    read_i=0;
  }
  mem=malloc(memsize); // getting ready for the big moment
  p=0;                 // what is p???
  codeptr=code;
  if(debug){
    printf("interpreting code:\n%s\n",code);
  }
  n=run(1); // here we go
  if(n==0){
    return 0;
  }else if(n>0){
    fprintf(stderr,"%d unbalanced []'s\n",n);
  }else if(n==-1){
    fputs("char overflow\n",stderr);
  }else if(n==-2){
    fputs("char underflow\n",stderr);
  }else if(n==-3){
    fputs("negative array pointer\n",stderr);
  }else if(n==-4){
    perror("brainfuck");
  }else{
    fputs("unknown error\n",stderr);
  }
  return 3;
}
