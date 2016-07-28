#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>

#define PORT 5000
typedef struct serversrecord{
  char name[32];
  char ip[32];
  int port;
  int online;
}SERVREC;
typedef struct clientrecord{
 char username[32];
 int fd,id;
 char room[32];
 int online;
}CLIREC;
CLIREC clirec[100];
 char msg[64];
int cid;
void *handleclient (void *s)
{
  int fd;
  int nrb;
  int num;
  int nowid;
  int i;
  char username[32];
  char msg[64];
  char buf[64];
  bzero(username,32);
  bzero(buf,64);
  bzero(msg,64);
  fd= *(int *)s;
  printf("%d\n",fd);
  printf("reader started\n");
  nowid=cid++;
  num=10;
  nrb=recv(fd,username,32,0);
  printf("connected user:%s\n",username);
  strcpy(clirec[nowid].username,username);
  clirec[nowid].online=1;
  clirec[nowid].fd=fd;
  clirec[nowid].id=nowid;
  while(clirec[nowid].fd!=0)
  { 
    //printf("file desc:%d\n",fd);
    nrb=read(fd,buf,64);
    if(nrb<=0)
     {
      if(nrb==0)
       {
        clirec[nowid].fd=0;
        clirec[nowid].online=0;
        printf("%s closed\n",username);
        }
        else
          continue;
      }
    else
        {
            bzero(msg,64);
            strcpy(msg,clirec[nowid].username);
            strcat(msg," said:");
            strcat(msg,buf);
          for(i=0;i<=cid;i++)
          {
            if((clirec[i].online==-1) || (clirec[i].id==nowid));
            else
            write(clirec[i].fd,msg,64);
          }
        }
   }
}
int main(int argc ,char * argv[])
{
	int client,thisserver,clientdesc;
    char welcome[32];
    pthread_t t;
    cid=1;
    int nrb,i;
    char username[32];
    char newroomname[]=argv[1];
    unsigned int port;
    port=atoi(argv[2]);
    unsigned int clinetaddrlen;
    SERVREC thisservrec;
    for(i=0;i<100;i++)
    {
        memset(&clirec[i],0,sizeof(clirec[i]));
        clirec[i].online=-1;
    }
	  char identfier[32]="i am server";
    struct sockaddr_in serveraddr,thisserveraddr,clientaddr;
    client=socket(AF_INET,SOCK_STREAM,0);//connecting withe mainserver
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serveraddr.sin_family=PF_INET;
    serveraddr.sin_port=htons(8888);
    if(connect(client,(struct sockaddr *) &serveraddr,sizeof(serveraddr))<0)
      perror("connect");
    thisserver=socket(AF_INET,SOCK_STREAM,0);//this chat room
    printf("%d\n",thisserver);
    memset(&thisserveraddr,0,sizeof(thisserveraddr));
    thisserveraddr.sin_addr.s_addr=INADDR_ANY;
    thisserveraddr.sin_family=PF_INET;
    thisserveraddr.sin_port=htons(PORT);
    strcpy(thisservrec.name,"inteserver");//this chat room record
    strcpy(thisservrec.ip,inet_ntoa(thisserveraddr.sin_addr));
    thisservrec.port=PORT;
    write(client,identfier,32);
    read(client,welcome,32);
    if(strcmp(welcome,"welcome server")==0)
    write(client,(SERVREC *) &thisservrec,sizeof(thisservrec));
    close(client);
    printf("done\n");
    int yes;yes=1;
    setsockopt(thisserver, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    printf("name:%s ip:%s port:%d\n",thisservrec.name,thisservrec.ip,thisservrec.port);
    if((bind(thisserver,(struct sockaddr *) &thisserveraddr,sizeof(thisserveraddr)))<0)
      perror("binding");
    listen(thisserver,10);
    while(1)
    {
        bzero(username,32);
        clinetaddrlen=sizeof(clientaddr);
        if((clientdesc=accept(thisserver,(struct sockaddr *) &clientaddr,&clinetaddrlen))<0)
          perror("accept");
          printf("clientdesc :%d\n",clientdesc );
          printf("accepted\n");
          pthread_create(&t,NULL,handleclient,(void *) &clientdesc);
    }
}

