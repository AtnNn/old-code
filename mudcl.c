/*
 * Mud client for medievia.com:4000
 * (C) 2003 AtnNn
 * software:	source, binary files, documentation and all other files
 * 				distributed with this program
 * The copyright holder denies all his rights to empeach you to modify and
 * redistribute this software in whole or in part, under one condition:
 * you must retain this text in its whole in order to make sure that subsequent
 * users have knowledge of the origin of this software. Any use of this software
 * who's purpose is to deny every man's right for knowledge is strongly
 * not recommended.
 *
 * WARRANTY
 *
 * THIS TEXT IS IN CAPS BECAUSE YOU SHOULD SCREAM WHEN YOU READ IT. THE
 * COPYWRITE HOLDER DENIES ALL RESPONSIBILITY FOR ANY BUGS, CRASHES, DENIAL OF
 * SERVICE AND INCLUDING BUT NOT EXCLUDING MISUSE, ABUSE AND EASTER EGGS.
 * MISTREATEMENT OF THIS PROGRAM CAN BE PURSUED IN COURT. FOR EXAMPLE, YOU
 * CANNOT BLAME THE COPYWRITE HOLDER FOR CODING THIS PROGRAM. YOU ARE THE ONLY
 * RESPONSIBLE, BECAUSE YOU WILL RUN IT, YOU WILL LOVE IT, YOU LOOOVE THIS
 * PROGRAM, REPEAT AFTER ME: I LOVE THIS SOFTWARE. FOR MORE INFORMATION PLEASE
 * CALL OUR HOTLINE.
 *
 * Medievia, Medweb, and other deriviatives are trademarks of medievia.com
 * unless where specified otherwise
 */
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#define SIZE 1024
#define LINES 16
struct termios oldi,oldo,new;
void curleft(int n){
	while(n--){
		write(1,"\x1B[D",3);
	}
}
void curright(int n){
	while(n--){
		write(1,"\x1B[C",3);
	}
}
void beep(){
	write(1,"\a",1);
}
void quit(int n){
	tcsetattr(0,TCSAFLUSH,&oldi);
	tcsetattr(1,TCSAFLUSH,&oldo);
	exit(n);
}
int main(int argc, char **argv){
	fd_set fdsm,fds;
	int sockfd,n,ipos,lpos;
	struct hostent *he;
	struct sockaddr_in si;
	char *ibuf,*sbuf,*c,**ilines,*tmp;
	if(!(he=gethostbyname("medievia.com"))){
		perror("gethostbyname");
		return -1;
	}
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("socket");
		return -1;
	}
	si.sin_family=AF_INET;
	si.sin_port=htons(4000);
	si.sin_addr=*(struct in_addr*)he->h_addr;
	memset(&si.sin_zero,0,8);
	if(connect(sockfd,(struct sockaddr*)&si,sizeof(struct sockaddr))==-1){
		perror("connect");
		return -1;
	}
	FD_ZERO(&fdsm);
	FD_SET(sockfd,&fdsm);
	FD_SET(0,&fdsm);
	fcntl(sockfd,F_SETFL,O_NONBLOCK);
	fcntl(0,F_SETFL,O_NONBLOCK);
	tcgetattr(0,&oldi);
	tcgetattr(1,&oldo);
	tcgetattr(0,&new);
	cfmakeraw(&new);
	tcsetattr(0,TCSANOW,&new);
	ibuf=malloc(SIZE);
	*ibuf=0;
	ipos=0;
	sbuf=malloc(SIZE);
	ilines=malloc(LINES*sizeof(char*));
	memset(ilines,0,LINES*sizeof(char*));
	*ilines=ibuf;
	lpos=0;
	for(;;){
		fds=fdsm;
		if(!select(sockfd+1,&fds,NULL,NULL,NULL)){
			perror("select");
			quit(-1);
		}
		if(FD_ISSET(sockfd,&fds)){
			for(;;){
				n=read(sockfd,sbuf,SIZE);
				if(n==-1){
					if(errno==EAGAIN)
						break;
					else{
						perror("socket read");
						quit(-1);
					}
				}else if(!n){
					write(1,"\r\nBYE!\r\n",8);
					quit(0);
				}
				write(1,sbuf,n);
			}
			write(1,ibuf,n=strlen(ibuf));
			curleft(n-ipos);
		}
		if(FD_ISSET(0,&fds)){
			for(;;){
				n=read(0,sbuf,SIZE);
				if(n==-1){
					if(errno==EAGAIN)
						break;
					else{
						perror("stdin read");
						quit(-1);
					}
				}
				//write(sockfd,ibuf,n);
				//write(1,ibuf,n);
				sbuf[n]=0;
				c=sbuf;
				while(*c){
					if(*c==13){
						write(1,"\n\r",2);
						n=strlen(ibuf);
						ibuf[n]='\n';
						write(sockfd,ibuf,n+1);
						ibuf[n]=0;
						if(lpos){
							 while(--lpos){
								ilines[lpos+1]=ilines[lpos];
							 }
							 *ilines=ibuf;
						}
						if(ibuf[0]){
							free(ilines[LINES-1]);
							for(n=LINES-1;n;n--){
								ilines[n]=ilines[n-1];
							}
							ibuf=malloc(SIZE);
							*ilines=ibuf;
						}
						ipos=0;
						*ibuf=0;
					}else if(*c==3){//ctrl+c
						quit(-1);
					}else if(*c==8){//backspace
						if(!ipos){
							beep();
							c++;
							continue;
						}
						curleft(1);
						write(1,ibuf+ipos,n=(strlen(ibuf)-ipos));
						write(1," ",1);
						curleft(n+1);
						ipos--;
						n=ipos;
						while(ibuf[n]){
							ibuf[n]=ibuf[++n];
						}
					}else if(*c==127){//delete
					}else if(*c==27){
						c++;
						if(!c){//escape
							break;
						}else if(*c==91){
							c++;
							//up:65, down:66 left: 68 right: 67
							if(*c==68){
								if(ipos){
									ipos--;
									curleft(1);
								}else{
									beep();
								}
							}else if(*c==67){
								if(ibuf[ipos]){
									ipos++;
									curright(1);
								}else{
									beep();
								}
							}else if(*c==65){
								if(lpos==LINES||!ilines[lpos+1]){
									beep();
									c++;
									continue;
								}
								curleft(ipos);
								lpos++;
								ibuf=ilines[lpos];
								write(1,ibuf,n=strlen(ibuf));
								ipos=n;
							}else if(*c==66){
								if(lpos==0){
									beep();
									c++;
									continue;
								}
								curleft(ipos);
								lpos--;
								ibuf=ilines[lpos];
								write(1,ibuf,n=strlen(ibuf));
								ipos=n;
							}
						}
					}else{
						if(strlen(ibuf)>=SIZE){
							beep();
							c++;
							continue;
						}
						write(1,c,1);
						write(1,ibuf+ipos,(n=strlen(ibuf))-ipos);
						curleft(n-ipos);
						for(;n>=ipos;n--){
							ibuf[n+1]=ibuf[n];
						}
						ibuf[ipos]=*c;
						ipos++;
					}
					c++;
				}
			}
		}
	}
	quit(0);
}
