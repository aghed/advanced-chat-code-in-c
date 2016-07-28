#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
  int serverconnection;
  int client;
  int changeroom;
typedef struct serversrecord{
  char name[32];
  char ip[32];
  int port;
  int online;
  int stellinroom;
}SERVREC;
typedef struct newroom
{
  char name[32];
  char port[32];
}NEWROOM;
int stellinroom;
pthread_t t1,t2;
int stringcmp(char str1[],char str2[])
{
  int i;
  int len;
  int res;
  len=strlen(str1);
  res=1;
  for(i=0;i<len;i++)
  {
   if((str1[i]!='\0') && (str2[i]!='\0'))
   if(str1[i]!=str2[i])
    {
      res=0;
      break;
    }
  }
  return res;
}
void *reader(void *s)
{
  int fd;
  int nrb;
  int num;
  num=10;
  char buf[64];
  fd=*(int *)s;
  while(fd!=0 && stellinroom)
  { 
      bzero(buf,64);
      nrb=read(fd,buf,64);
    if(nrb<=0)
     {
      if(nrb==0)
      {
        fd=0;
        printf("server closed\n");
        stellinroom=0;
        pthread_cancel(t1);
        pthread_cancel(t2);
      }
      else 
          usleep(10);
     }
      else
      printf("%s",buf);
  }
}
void *writer(void *s)
{
  int fd;
  int nrb;
  int num;
  num=10;
  changeroom=0;
  char buf[64];
  fd=*(int *)s;
  while(stellinroom)
  {
    bzero(buf,64);
    fgets(buf,64,stdin);
    if(strlen(buf)<=1)
      usleep(10);
    else
   {
     if(stringcmp(buf,"Disconnect"))
    {
      stellinroom=0;
      close(serverconnection);
      pthread_cancel(t2);
      pthread_cancel(t1);
    }
    else if(stringcmp(buf,"ChangeRoom"))
      {
      stellinroom=0;
      changeroom=1;
      close(serverconnection);
      printf("changeing room\n");
      pthread_cancel(t2);
      pthread_cancel(t1);   
      }
    else
      write(fd,buf,64);
   } 
  }
}

int main(int argc ,char * argv[])
{
  int nrb;
  int iplen;
  NEWROOM nr;
  SERVREC roomserver;
  struct hostent *he;
  struct in_addr ** addr_list;
  memset(&roomserver,0,sizeof(roomserver));
    char wel[32];
    char username[32];
    char room[32];
    char availablerooms[1024];
    char commands[256];
    char command[32];
    char commandstatus[32];
    char useres[1024];
    char url[64];
    char urlhelp[64];
    char ip[32];
    char iphelper[32];
    char port[32];
    char roomfound[]="room found";
    char roomnotfound[]="room not found";
    char invalidcommand[]="invalid command";
    char listroom[]="listingrooms";
    char listuseres[]="usereslisting";
    char joinroom[]="chatadd";
    char createroom[]="createroom";
    char ok[]="ok";
    char ChangeRoom[]="JoinRoom";
    char roomstate[32];
    bzero(username,32);
    bzero(availablerooms,1024);
    bzero(commands,256);
    bzero(wel,32);
    bzero(roomstate,32);
    memset(&nr,0,sizeof(nr));
    char identfier[32]="i am client";
    printf("enter your name:");
    scanf("%s",username);
    struct sockaddr_in serveraddr,roomserveraddr;
    client=socket(AF_INET,SOCK_STREAM,0);
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serveraddr.sin_family=PF_INET;
    serveraddr.sin_port=htons(8888);
    connect(client,(struct sockaddr *) &serveraddr,sizeof(serveraddr));
    perror("connect()");
    write(client,identfier,32);
    read(client,wel,32);
    if(strcmp(wel,"welcome client")==0)
          write(client,username,32);
        printf("username writen\n");
    read(client,commands,256);
    printf("%s\n",commands);
    printf("you can type 'quit' to quit the program\n");
    while(1)
    {
    bzero(command,32);  
    bzero(commandstatus,32);
    printf("choose command:");
    if(changeroom)
    {
      write(client,ChangeRoom,32);
      changeroom=0;
    }
    else
    {
     fgets(command,32,stdin);/////////////////////handling commands  /reading the command
     if(stringcmp(command,"quit"))
     break;
     write(client,command,32);// sending the command
     }
     read(client,commandstatus,32);// reading the response
     printf("%s\n",commandstatus);
     if(stringcmp(commandstatus,invalidcommand))
     {
      printf("wrong command\n");
      continue;
     }
    else if(stringcmp(listroom,commandstatus))
    {
    read(client,availablerooms,1024);
    printf("%s",availablerooms);
     }
     else if(stringcmp(joinroom,commandstatus))
    {

     printf("enter room name:\n");
     scanf("%s",room);
     write(client,room,32);
     read(client,roomstate,32);
     write(client,ok,32);
     if(stringcmp(roomstate,roomfound))
     {
       int nread;
       bzero(url,64);
       bzero(urlhelp,64);
       bzero(ip,32);
       bzero(port,32);
       bzero(iphelper,32);
     if(nread=read(client,url,64)<0)
       perror("read");
     printf("%s\n",url);
     strcpy(urlhelp,url);
     strcpy(ip,strtok(urlhelp,":"));
     iplen=strlen(ip);
     strcpy(port,strtok(url+iplen+1,""));
     if((he=gethostbyname(ip))==NULL)
      strcpy(iphelper,"0.0.0.0");
     else
     {
      addr_list=(struct in_addr ** )he->h_addr_list;
      strcpy(iphelper,inet_ntoa(* addr_list[0]));
     }
     printf("roomserver ip:%s roomserver port:%s\n",iphelper,port);
     serverconnection=socket(AF_INET,SOCK_STREAM,0);
     memset(&roomserveraddr,0,sizeof(roomserveraddr));
     roomserveraddr.sin_addr.s_addr=inet_addr(iphelper);
     roomserveraddr.sin_port=htons(atoi(port));
     roomserveraddr.sin_family=PF_INET;
     connect(serverconnection,(struct sockaddr *) &roomserveraddr,sizeof(roomserveraddr));
     perror("connect()");
       printf("connected to chatroom\n");
       printf("%s\n",username);
       nrb=send(serverconnection,username,32,0);
       if(nrb<0)
       {
         printf("failed\n");
         perror("write");
       }
       else  
        printf("sent\n");
        stellinroom=1;
        while(stellinroom)
      {perror("connect to the mainserver()");
        printf("%d\n",stellinroom);
       pthread_create(&t2,NULL,writer,(void *) &serverconnection);
       pthread_create(&t1,NULL,reader,(void *) &serverconnection);
       pthread_join(t1,(void *)&stellinroom);
       pthread_join(t2,(void *)&stellinroom);
      }
      printf("threads canceled\n");
      usleep(10);
      printf("%d\n",changeroom);
      printf("%s\n",ChangeRoom);
     }
     else
      printf("invalid room name\n");
    }
    else if(stringcmp(listuseres,commandstatus))
    {
      bzero(useres,1024);
      read(client,useres,1024);
      printf("%s\n",useres);
    }
    else if(stringcmp(createroom,commandstatus))
    {    
      printf("enter room name:");
      fgets(nr.name,32,stdin);
      printf("enter room port:");
      fgets(nr.port,32,stdin);
      write(client,(NEWROOM *) &nr,sizeof(nr)); 
    }
  }
  printf("bye bye -_- :)\n");
}
