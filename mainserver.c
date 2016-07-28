#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>

#define N 20
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
typedef struct newroom
{
  char name[32];
  char port[32];
}NEWROOM;
SERVREC servrec[N];
CLIREC clirec[100];
int cid,sid;
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
void rebuildclifile()
{
  int i;
  char helpid[5];
  char buf[1024];
  bzero(buf,1024);
  for(i=1;i<=cid;i++)
   {
    if(clirec[i].online)
    {
      bzero(helpid,5);
      sprintf(helpid,"%d",clirec[i].id);
      strcat(buf,helpid);
      strcat(buf,"          ");
      strcat(buf,clirec[i].username);
      strcat(buf,"        ");
      strcat(buf,clirec[i].room);
      strcat(buf,"\n");
    }
   }
   FILE *fpc;
   fpc=fopen("clients_file.txt","w");
   fprintf(fpc,"%s\n","client_id  client_name  client_room" );
   fwrite(buf,1,strlen(buf),fpc);
   fclose(fpc);
}
void rebuildservfile()
{
 int i;
 char helpid[5];
 char buf[1024];
 bzero(buf,1024);
  for(i=1;i<=sid;i++)
   {
    if(servrec[i].online)
    {
      strcat(buf,servrec[i].name);
      strcat(buf,"         ");
      strcat(buf,servrec[i].ip);
      strcat(buf,"       ");
      sprintf(helpid,"%d",servrec[i].port);
      strcat(buf,helpid);
      strcat(buf,"\n");
    }
   }
   FILE *fps;
   fps=fopen("servers_file.txt","w");
   fprintf(fps, "%s\n","chatrrom_name  chatroom_ip  chatroom_port" );
   fwrite(buf,1,strlen(buf),fps);
   fclose(fps);
}
void *handleclient(void *s)
{

  int clientdesc,i,numby;
  int nowid;
  int nrb;
  FILE *fpc,*fps;
  NEWROOM nr;
  nowid=cid++;
  char username[32];
  char room[32];
  char newroomname[32];
  int foundroom;//for requested room search
  foundroom=0;
  char roomfound[]="room found";
  char roomnotfound[]="room not found";
  char createnewroom[32];
  char commands[256];
  char command[32];
  char ok[32];
  char url[64];
  char forfiles[1024];
  char invalidcommand[]="invalid command";
  char listroom[]="listingrooms";
  char listuseres[]="usereslisting";
  char joinroom[]="chatadd";
  char createroom[]="createroom";
  char availblerooms[1024];
  char users[1024];
  char msg[32];
  char welcomeclient[32]="welcome client";
  char helpid[5];
  bzero(username,32);
  bzero(availblerooms,1024);
  bzero(users,1024);
  bzero(commands,256);
  bzero(room,32);
  bzero(msg,32);
  bzero(command,32);
  bzero(createnewroom,32);
  bzero(ok,32);
  bzero(url,64);
  bzero(forfiles,1024);
  memset(&nr,0,sizeof(nr));
  strcat(commands,"you can:\n");
  strcat(commands," 1-ShowRooms\n");
  strcat(commands," 2-ListUseres\n");
  strcat(commands," 3-JoinRoom\n");
  strcat(commands," 4-CreateNewRoom\n");
  strcat(commands," 5-ChangeRoom\n");
  strcat(commands,"please be aware of capital letteres and 'nospaces'");
  strcat(availblerooms,"availblerooms are:\n");
  pthread_t t1,t2;
  clientdesc=*(int *)s;
  write(clientdesc,welcomeclient,32);
  read(clientdesc,username,32);
  printf("%s\n",username);
  clirec[nowid].id=nowid;
  strcpy(clirec[nowid].username,username);
  clirec[nowid].online=1;
  rebuildclifile();
  int len;
  len=strlen(commands);
  printf("%d\n",len);
  write(clientdesc,commands,256);
  printf("commands writen\n");
  bzero(command,32);
while(clirec[nowid].online)
{  
  bzero(command,32);
  nrb=read(clientdesc,command,32);
  if(nrb==0)
  {
    memset(&clirec[nowid],0,sizeof(clirec[nowid]));
    clirec[nowid].online=0;
    rebuildclifile();
    pthread_exit(NULL);
  }
  printf("%s\n",command);
  if(stringcmp(command,"ShowRooms"))
  {
    printf("listroom\n");
    write(clientdesc,listroom,32);
    fps=fopen("servers_file.txt","r");
    if(fps==NULL)
      fprintf(stderr, "%s\n","failed to open" );
    fread(forfiles,1024,1,fps);
    fclose(fps);
   strcat(availblerooms,forfiles);
   write(clientdesc,availblerooms,1024);
   bzero(availblerooms,1024);
   bzero(forfiles,1024);
  }
else
  if(stringcmp(command,"ListUseres"))
  {
    write(clientdesc,listuseres,32);
    fpc=fopen("clients_file.txt","r");
    fread(users,1024,1,fpc);
    fclose(fpc);
    write(clientdesc,users,1024);
    bzero(users,1024);
  }
else
  if(stringcmp(command,"JoinRoom"))
{
  write(clientdesc,joinroom,32);
  read(clientdesc,room,32);
  printf("chossen room:%s\n",room);
  for(i=1;i<=sid;i++)
  {
    if(strcmp(servrec[i].name,room)==0)
    {
      foundroom=1;
      printf("found\n");
      break;
    }
  }
  if(foundroom)
  {
      write(clientdesc,roomfound,32);
      read(clientdesc,ok,32);
      strcat(url,"www.");
      strcat(url,servrec[i].name);
      strcat(url,".com:");
      sprintf(helpid,"%d",servrec[i].port);
      strcat(url,helpid);
      write(clientdesc,url,64);
      strcpy(clirec[nowid].room,room);
      printf("writen\n");
      rebuildclifile();
  }
  else 
    {
      write(clientdesc,roomnotfound,32);
      read(clientdesc,ok,32);
}
}
else
  if(stringcmp(command,"CreateNewRoom"))
  {
    printf("%s\n",command);
    write(clientdesc,createroom,32);
   //write(clientdesc,newroommsg,64);
   read(clientdesc,(NEWROOM *) &nr,sizeof(nr));
   strcat(createnewroom,"gnome-terminal -e './new ");
   strcat(createnewroom,nr.name);
   strcat(createnewroom," ");
   strcat(createnewroom,nr.port);
   strcat(createnewroom," ' ");
   system(createnewroom); 
  }
else
{
  write(clientdesc,invalidcommand,32);
}
 }
}
void *handleserver(void *p)
{
  printf("the sid:%d\n",sid);
  int s=*(int *) p;
  int nowid;
  int nrb;
  int i;
  char buf[32];
  char closemsg[64];
  bzero(closemsg,64);
  nowid=sid++;
  SERVREC thisserver;
  bzero(buf,32);
  memset(&thisserver,0,sizeof(thisserver));
  char welcomeserver[]="welcome server";
  write(s,welcomeserver,strlen(welcomeserver));
  read(s,(SERVREC *) &thisserver,sizeof(thisserver));
  thisserver.online=1;
  memcpy(&servrec[nowid],&thisserver,sizeof(thisserver));
    printf("chatroom %s online:\n",servrec[nowid].name);
    printf("chatroom ip:%s chatroom port:%d\n",servrec[nowid].ip,servrec[nowid].port);
    rebuildservfile();
  while(servrec[nowid].online)
  {
      bzero(buf,32);
      nrb=read(s,buf,32);
      if(nrb<=0)
      {
        if(nrb==0)
      {
        servrec[nowid].online=0;
        printf("chatroom %s closed\n",servrec[nowid].name);
        memset(&servrec[nowid],0,sizeof(nowid));
        for(i=1;i<=sid;i++)
        {
          if(stringcmp(clirec[i].room,servrec[nowid].name))
            bzero(clirec[i].room,32);
        }
        rebuildservfile();
        rebuildclifile();
        pthread_exit(NULL);
      }
      }
      else 
      {
        for(i=1;i<=sid;i++)
        {
          if(stringcmp(clirec[i].username,buf))
          {
           bzero(clirec[i].room,32);
           rebuildclifile();
           break;
          }
        }
      }
  }
}
int main(int argc,char * argv[])

{
   sid=1,cid=1;
   int i;
   FILE *fps,*fpc;
   for (i=1;i<N;i++)
    memset(&servrec[i],0,sizeof(servrec[i]));
   char welcomeclient[]="welcome client";
   SERVREC servrec[N];
  int server,client,clientlen;
  char msg[32];
  bzero(msg,32);
  remove("clients_file.txt");
  remove("servers_file.txt");
  fps=fopen("servers_file.txt","w");
  fclose(fps);
  fpc=fopen("clients_file.txt","w");
  fclose(fpc);
  pthread_t clithread,serthread;
  struct sockaddr_in serveraddr,clientaddr;
  server=socket(PF_INET,SOCK_STREAM,0);
  memset(&serveraddr,0,sizeof(serveraddr));
  serveraddr.sin_addr.s_addr=INADDR_ANY;
  serveraddr.sin_port=htons(8888);
  serveraddr.sin_family=AF_INET;
  int yes;yes=1;
  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  bind(server,(struct sockaddr*) &serveraddr,sizeof(serveraddr));
  perror("bind():");
  listen(server,50);
  perror("listen():");
  for(;;)
  {
    clientlen=sizeof(clientaddr);
    client=accept(server,(struct sockaddr*) &clientaddr,&clientlen);
    printf("accepted\n");
    read(client,msg,32);
     if(strcmp(msg,"i am client")==0)
     {
        printf("yes\n");
        pthread_create(&clithread,NULL,handleclient,(void *) &client);
      }
      else
        pthread_create(&serthread,NULL,handleserver,(void *) &client);
  }
}