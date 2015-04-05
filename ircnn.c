// Written by Etienne Laurin

// Bored my the limited world of mirc scripting
// I set out to write my own client

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

//struct mysockets -> type
#define IRCCLIENT 1
#define DCCCHAT 2
#define DCCSEND 3
#define DCCRECEIVE 4

//adjust these if necesary
#define MAXDATASIZE 256
#define BIGBUFSIZE 8192

#define CONFIG_FILE "ircnn.conf"
#define COMMANDCHAR '!'

struct mysockets { int sockfd; int type; char *recvbuf; struct mysockets *next; } *connections=NULL;
struct myadmins { char *nick,*ident,*host,*pass,*flags; struct myadmins *next; } *admins=NULL;
struct mychannels { char *name,*flags,*key; struct myadmins *admins,*identified; struct mychannels *next; };
struct myservers { char *name,*nick; struct mychannels *chans; struct myadmins *admins,*identified; int socket; FILE *fp; int port; struct myservers *next; } *servers=NULL;
fd_set readfds;
int maxsockfd=0;

void loadconfig(void);
int ircconnect(char *server, int port);
void recved(char *buf,struct myservers *server);
void freeconfig(void);
void freeadmins(struct myadmins *admin);
void freechans(struct mychannels *chan);
void freeservers(struct myservers *server);

char mytoupper(char c){
	return c>'z'||c<'a'?c:c-32;
}

int mystrcmp(char*a,char*b){
	int y;
 	for(y=0;a[y]||b[y];y++){
 		if(mytoupper(a[y])!=mytoupper(b[y])){
 			y=0;
 			break;
 		}
 	}
	if(!(a[y]&&b[y])){
		return 1;
	}
	return 0;
}

//checks if s2 is similar to s2, using very basic wildcards
int simstr(char *string, char *match){
	int y;
	printf("simstr(%s,%s)=",string,match);
	if(!strcmp(match,"*")||!strcmp(match,"-")){
		printf("true\n");
 		return 1;
 	}
	if(mystrcmp(string,match)){
		printf("true\n");
		return 1;
	}
	printf("false\n");
 	return 0;
}

//check highest level of who in linked list admin
int hasflags(struct myadmins who, struct myadmins *admin, char *flags){
	char *t;
	for(;admin;admin=admin->next){
		if(!strcmp(who.nick,admin->nick)&&!strcmp(who.ident,admin->ident)&&!strcmp(who.host,admin->host)){
			for(t=flags;*t;t++)
				if(strchr(admin->flags,*t))
					return 1;
		}
	}
	return 0;
}

//my own strdup :P
char *dupstr(char *s){
	char *ret;
	ret=malloc(sizeof(char)*(strlen(s)+1));
	strcpy(ret,s);
	return ret;
}

//counts number of c's in s
int chrcount(char *s, char c){
	int ret=0;
	while(*s){
		if(*s==c)
			ret++;
		s++;
	}
	return ret;
}

//trims off trailing and leading space characters
char *trim(char *str)
{
        int i=0;
        if(str) {
                while(str[i]==' '||str[i]=='\t'||str[i]=='\n'||str[i]=='\r')
                        i++;
                str = str + i;
                for(i=strlen(str)-1; i > -1; i--) {
                        if(str[i]==' '||str[i]=='\t'||str[i]=='\n'||str[i]=='\r')
                                str[i] = 0;
                        else
                                break;
                }
        }
        return str;
}

//do the specified command
void docommand(char *what,struct myadmins who,struct myservers *server, char *to){
	struct mychannels *chan;
	struct myadmins *admin,*newadmin;
	int tmp;
	if(who.pass==NULL)
		who.pass="-";
	for(chan = server->chans;chan;chan=chan->next)
		if(!strcmp(chan->name,to))
			break;
	if(beginc(what,"quit")){
		if(hasflags(who,admins,"RA")||hasflags(who,server->identified,"RA")){
			what+=4;
			fprintf(server->fp,"QUIT :%s\n",*what?"requested":what+1);
			printf("QUIT :%s\n",*what?what+1:"requested");
		}else{
			fprintf(server->fp,"NOTICE %s :You do not have the right privilieges to use !quit\n",who.nick);
			printf("NOTICE %s :You do not have the right privilieges to use !quit\n",who.nick);
		}
	}else if(beginc(what,"green ")){
		fprintf(server->fp,"PRIVMSG %s :%c9,1%s\n",to,3,what+6);
		printf("PRIVMSG %s :%c9,1%s\n",to,3,what+6);
	}else if(beginc(what,"join ")){
		if(hasflags(who,server->identified,"RA")){
			what+=5;
			fprintf(server->fp,"JOIN %s\n",what);
			printf("JOIN %s\n",what);
		}else{
			fprintf(server->fp,"NOTICE %s :You do not have the right privilieges to use !join\n",who.nick);
			printf("NOTICE %s :You do not have the right privilieges to use !join\n",who.nick);
		}
	}else if(beginc(what,"raw ")){
		if(hasflags(who,server->identified,"RA")){
			what+=4;
			fprintf(server->fp,"%s\n",what);
			printf("%s\n",what);
		}else{
			fprintf(server->fp,"NOTICE %s :You do not have the right privilieges to use !raw\n",who.nick );
			printf("NOTICE %s :You do not have the right privilieges to use !raw\n",who.nick );
		}
	}else if(beginc(what,"op ")&&chan!=NULL){
		if(hasflags(who,chan->identified,"oO")||hasflags(who,server->identified,"oO")){
			what+=3;
			fprintf(server->fp,"MODE %s +o %s\n",to,what);
			printf("MODE %s +o %s\n",to,what);
		}
	}else if(!strcmp(what,"op")&&chan!=NULL){
		if(hasflags(who,chan->identified,"oO")||hasflags(who,server->identified,"oO")){
			what+=3;
			fprintf(server->fp,"MODE %s +o %s\n",to,who.nick);
			printf("MODE %s +o %s\n",to,who.nick);
		}
	}else if(beginc(what,"deop ")&&chan!=NULL){
		if(hasflags(who,chan->identified,"oO")||hasflags(who,server->identified,"oO")){
			what+=5;
			fprintf(server->fp,"MODE %s -o %s\n",to,what);
			printf("MODE %s -o %s\n",to,what);
		}
	}else if(!strcmp(what,"deop")&&chan!=NULL){
		if(hasflags(who,chan->identified,"oO")||hasflags(who,server->identified,"oO")){
			what+=5;
			fprintf(server->fp,"MODE %s -o %s\n",to,who.nick);
			printf("MODE %s -o %s\n",to,who.nick);
		}
	}else if(!strcmp(what,"checkme ")&&chan!=NULL){
		what+=8;
		if(hasflags(who,chan->identified,what)){
			fprintf(server->fp,"PRIVMSG %s :you have the flags %s in %s",to,what,to);
			printf("PRIVMSG %s :you have the flags %s in %s",to,what,to);
		}
		if(hasflags(who,server->identified,what)){
			fprintf(server->fp,"PRIVMSG %s :you have the flags %s on this server",to,what);
			printf("PRIVMSG %s :you have the flags %s on %s",to,what,server->name);
		}
	}else if(beginc(what,"login ")&&chan!=NULL){
		what+=6;
		tmp=0;
		for(admin=admins;admin;admin=admin->next){
			if(simstr(who.nick,admin->nick)&&simstr(who.ident,admin->ident)&&simstr(who.host,admin->host)&&!strcmp(what,admin->pass)){
				tmp=1;
				break;
			}
		}
		if(!tmp){
			for(admin=server->admins;admin;admin=admin->next){
				if(simstr(who.nick,admin->nick)&&simstr(who.ident,admin->ident)&&simstr(who.host,admin->host)&&!strcmp(what,admin->pass)){
					tmp=1;
					break;
				}
			}
		}
		if(!tmp){
			for(admin=chan->admins;admin;admin=admin->next){
				if(simstr(who.nick,admin->nick)&&simstr(who.ident,admin->ident)&&simstr(who.host,admin->host)&&!strcmp(what,admin->pass)){
					tmp=2;
					break;
				}
			}
		}
		if(tmp){
			newadmin=malloc(sizeof(struct myadmins));
			newadmin->nick=dupstr(who.nick);
			newadmin->ident=dupstr(who.ident);
			newadmin->host=dupstr(who.host);
			newadmin->pass=dupstr(admin->pass);
			newadmin->flags=dupstr(admin->flags);
			newadmin->next=tmp==1?server->identified:chan->identified;
			tmp==1?server->identified:chan->identified=newadmin;
			fprintf(server->fp,"NOTICE %s :You have successfully identified\n",who.nick);
			printf("NOTICE %s :You have successfully identified\n",who.nick);
		}else{
			fprintf(server->fp,"NOTICE %s :Wrong hostmask or password\n",who.nick);
			printf("NOTICE %s :Wrong hostmask or password\n",who.nick);
		}
	}
	fflush(server->fp);
}

//when something has arrived on a IRC type socket
void ircrecv(struct mysockets *conn){
	char buf[MAXDATASIZE];
	struct myservers *server;
	int i,t;
	int numbytes;
	server = servers;
	while(server!=NULL&&server->socket!=conn->sockfd)
		server=server->next;
	if(server==NULL)
		return;
	numbytes=recv(conn->sockfd, buf, MAXDATASIZE-1, 0);
	buf[numbytes] = '\0';
	//printf("<recv> %s",buf);
	if(strlen(conn->recvbuf)+strlen(buf)>=BIGBUFSIZE){
		return;
	}
	strcat(conn->recvbuf,buf);
	t=strlen(conn->recvbuf);
	i=0;
	while(i<=t){
		if(conn->recvbuf[i]=='\n'||conn->recvbuf[i]=='\r'){
			conn->recvbuf[i]=0;
			//printf("%s\n",conn->recvbuf);
			recved(conn->recvbuf,server);
			if(conn->recvbuf[i+1]=='\n'||conn->recvbuf[i+1]=='\r')
				i++;
			strcpy(conn->recvbuf,conn->recvbuf+i+1);
			t=strlen(conn->recvbuf);
			i=-1;
		}
		i++;
	}
}

//case insensitive check if b begins with s
int beginc(char*b,char*s){
	int i=0;
	while(s[i]!=0&&b[i]!=0){
		if(toupper(b[i])!=toupper(s[i]))
			return 0;
		i++;
	}
	return 1;
}

//when a full line is found in server's connection->recvbuf it is parsed here
void recved(char *buf,struct myservers *server){
	//char sbuf[MAXDATASIZE];
	char *to;
	struct myadmins who;
	struct mychannels *chan;
	int i=0;
	if(beginc(buf,"ping")){
		if(!server->fp){
			server->fp=fdopen(server->socket,"w");
			fprintf(server->fp,"PONG %s\n",buf+6);
			for(chan=server->chans;chan;chan=chan->next){
				fprintf(server->fp,"JOIN %s %s\n",chan->name,chan->key);
			}
		}else{
			fprintf(server->fp,"PONG %s\n",buf+6);
			printf("PING? PONG!\n");
		}
	}else if(*buf==':'){
		buf++;
		to=strsep(&buf," ");
		who.pass="-";
		if(strchr(to,'!')){
			who.nick=strsep(&to,"!");
			who.ident=strsep(&to,"@");
			who.host=to;
		}else{
			who.nick=to;
			who.ident=NULL;
			who.host=NULL;
		}
		if(beginc(buf,"PRIVMSG ")){
			buf+=8;
			to=buf;
			i=0;
			while(buf[i]!=' '){
				if(buf[i]==0){
					printf("[ERROR]%s\n",buf);
					return;
				}
				i++;
			}
			buf[i]=0;
			buf+=i+2;
			printf("<%s:%s:%s> %s\n",who.nick,server->name,to,buf);
			if(*buf==COMMANDCHAR&&buf[1]!=0){
				docommand(buf+1,who,server,*to=='#'?to:who.nick);
			}else if(beginc(buf,"\x01PING ")){
				fprintf(server->fp,"NOTICE %s :%s\n",who.nick,buf);
			}else if(beginc(buf,"\x01VERSION")){
				fprintf(server->fp,"NOTICE %s :\x01VERSION noyb\x01\n",who.nick);
			}
		}else if(beginc(buf,"NOTICE AUTH")){
			printf("%s\n",buf+13);
		}else{
			printf("[UNKNOWN:]%s\n",buf);
		}
	}else{
		printf("[UNKNOWN]%s\n",buf);
	}
	fflush(server->fp);
}

//some say everything starts here
int main(int argc, char *argv[]){
	char buf[MAXDATASIZE];
	struct myservers *server;
	struct mysockets *conn;
	struct mychannels *chan;
	fd_set myfds;
	loadconfig();
	server=servers;
	while(server!=NULL){
		if(server->socket=ircconnect(server->name,server->port)){
			printf("successfully connected to %s\n",server->name);
			send(server->socket,"NICK ",5,0);
			send(server->socket,server->nick,strlen(server->nick),0);
			send(server->socket,"\nUSER ircnn ircnn ircnn :ircnn\n",31,0);
		}
		else
			printf("error connecting to %s\n",server->name);
		server=server->next;
	}
	for(;;){
		myfds=readfds;
		if(!select(maxsockfd+1,&myfds,NULL,NULL,NULL)){
			//select failed
		}
		conn=connections;
		while(conn!=NULL){
			if(FD_ISSET(conn->sockfd,&myfds)){
				if(conn->type=IRCCLIENT)
					ircrecv(conn);
			}
			conn=conn->next;
		}
	}
	freeconfig();
	return 0;
}

//load the config file into my linked lists of admins, servers, and channels
//see example ircnn.conf file
void loadconfig(void){
	FILE *fp;
	char buf[MAXDATASIZE];
	char *bp,*bg;
	int line=0,i;
	struct myadmins *admin=NULL;
	struct myservers *server=NULL;
	struct mychannels *chan=NULL;
	fp=fopen(CONFIG_FILE,"r");
	while(!feof(fp)){
		line++;
		fgets(buf,MAXDATASIZE,fp);
		bg=trim(buf);
		//printf(">%s\n",bg);
		if(*bg=='\0'||*bg==';')
			continue;
		if(bg[1]!=':'){
			fprintf(stderr,"Not in char + ':' format: line %d of %s ignored\n",line,CONFIG_FILE);
			continue;
		}
		bp=bg+2;
		switch(*bg){
		case 'a':
			if(chrcount(bp,':')!=2){
				fprintf(stderr,"a: not enough arguments: line %d of %s ignored\n",line,CONFIG_FILE);
				continue;
			}
			if(admin==NULL){
				if(server==NULL){
					admins = malloc(sizeof(struct myadmins));
					admin=admins;
				}else if(chan==NULL){
					server->admins = malloc(sizeof(struct myadmins));
					admin=server->admins;
				}else{
					server->chans->admins = malloc(sizeof(struct myadmins));
					admin=server->chans->admins;
				}
			}else{
				admin->next = malloc(sizeof(struct myadmins));
				admin = admin->next;
			}
			bg=strsep(&bp,":");
			admin->nick=dupstr(strsep(&bg,"!"));
			admin->ident=dupstr(strsep(&bg,"@"));
			admin->host=dupstr(bg);
			admin->pass=dupstr(strsep(&bp,":"));
			admin->flags=dupstr(bp);
			admin->next=NULL;
			break;
		case 's':
			if(chrcount(bp,':')!=2){
				fprintf(stderr,"s: not enough arguments: line %d of %s ignored\n",line,CONFIG_FILE);
				continue;
			}
			if(server==NULL){
				servers = malloc(sizeof(struct myservers));
				server=servers;
			}else{
				server->next = malloc(sizeof(struct myservers));
				server=server->next;
			}
			server->name=dupstr(strsep(&bp,":"));
			server->port=atoi(strsep(&bp,":"));
			if(!server->port)server->port=6667;
			server->nick=dupstr(bp);
			if(*bp=='-')server->nick="IrcNn";
			server->chans=NULL;
			server->admins=NULL;
			server->socket=0;
			server->identified=NULL;
			server->next=NULL;
			server->fp=NULL;
			chan=NULL;
			admin=NULL;
			break;
		case 'c':
			if(chrcount(bp,':')!=2||server==NULL){
				fprintf(stderr,"c: not enough arguments: line %d of %s ignored\n",line,CONFIG_FILE);
				continue;
			}
			if(chan==NULL){
				server->chans = malloc(sizeof(struct mychannels));
				chan=server->chans;
			}else{
				chan->next = malloc(sizeof(struct mychannels));
				chan=chan->next;
			}
			chan->name=dupstr(strsep(&bp,":"));
			chan->key=dupstr(strsep(&bp,":"));
			chan->flags=dupstr(bp);
			chan->next=NULL;
			chan->identified=NULL;
			chan->admins=NULL;
			admin=NULL;
			break;
		case 'j':
			//not yet implemented
			//break;
		default:
			fprintf(stderr,"Invalid beginning char: line %d of %s ignored\n",line,CONFIG_FILE);
		}
	}
	fclose(fp);
}

//free all the linked lists that were alloced
void freeconfig(void){
	freeadmins(admins);
	freeservers(servers);
}

void freeadmins(struct myadmins *admin){
	if(admin){
		freeadmins(admin->next);
		free(admin->nick);
		free(admin->ident);
		free(admin->host);
		free(admin->pass);
		free(admin->flags);
		free(admin);
	}
}

void freechans(struct mychannels *chan){
	if(chan){
		freechans(chan->next);
		free(chan->name);
		free(chan->flags);
		free(chan->key);
		freeadmins(chan->admins);
		free(chan);
	}
}

void freeservers(struct myservers *server){
	if(server){
		freeservers(server->next);
		free(server->name);
		free(server->nick);
		freechans(server->chans);
		freeadmins(server->admins);
		fclose(server->fp);
		free(server);
	}
}

//connect to irc server on port
int ircconnect(char *server, int port){
	struct hostent *he;
	struct sockaddr_in their_addr;
	//mysockets { int sockfd; int type; char *recvbuf; struct mysockets *next; }
	struct mysockets *t;
	if(connections==NULL){
		connections=malloc(sizeof(struct mysockets));
		t=connections;
	}else{
		t=connections;
		while(t->next!=NULL)
			t=t->next;
		t->next=malloc(sizeof(struct mysockets));
		t=t->next;
	}
	t->type=0;
	t->next=NULL;
	t->recvbuf=malloc(sizeof(char)*BIGBUFSIZE);
	if ((he=gethostbyname(server))==NULL){
		return 0;
	}
	if ((t->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return 0;
	}
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(port);
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);
	if (connect(t->sockfd, (struct sockaddr *)&their_addr,sizeof(struct sockaddr)) == -1) {
		return 0;
	}
	FD_SET(t->sockfd,&readfds);
	if(maxsockfd<t->sockfd)
		maxsockfd=t->sockfd;
	t->type=IRCCLIENT;
	return t->sockfd;
}
