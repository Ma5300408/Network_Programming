#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<string.h>
#include<netinet/in.h>
#include<time.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>
#include<fcntl.h>
#include<pthread.h>

#define MAXLINE 1500

void dg_echo(int,struct sockaddr*,socklen_t);

char **argv_gbl;

int main(int argc,char ** argv)
{
    int sockfd;
    struct sockaddr_in servaddr,cliaddr;
  
    if(argc != 3){
      printf("Enter <port number> <file name>\n");
      return 0;
    }
   
    if( (sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
       printf("Socket creation is wrong\n");
       return 0;
    }
     int i;
    argv_gbl = (char**)malloc(sizeof(char*)*argc);
    for(i = 0; i < argc; i++){
      argv_gbl[i] = (char*)malloc(sizeof(char)*MAXLINE);
    }
   for(i = 0; i < argc; i++){
      memcpy(argv_gbl[i],argv[i],strlen(argv[i])*4);
    }
   
   memset(&servaddr,0,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(atoi(argv[1]));
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

   if(bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0){
      printf("Bind is error");
      return 0;
   }
   
   dg_echo(sockfd,(struct sockaddr*)&cliaddr,sizeof(cliaddr));

}


void dg_echo(int sockfd,struct sockaddr* pcliaddr,socklen_t clilen)
{
   FILE *from;
    
     from = fopen(argv_gbl[2],"rb");
   if(from == NULL){
      printf("File not exist");
      return;
    }
   int i;
   int MAX = MAXLINE;
   struct timeval tv;
   int nrecv,nread,ifconnect;
   char seq_num[10];
   char recv_buf[MAXLINE],send_buf[MAXLINE],buff[MAXLINE];
   long fSize;
   int ACK_BUFFER_SIZE;
   int *ACK;
   int index;
   nread = 0;
   nrecv = -1;
   fseek(from,0,SEEK_END);
   fSize = ftell(from);
   rewind(from);
   ifconnect = 0;
   tv.tv_sec = 0;
   tv.tv_usec = 20;
   setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
   int seq;
   seq = 0;
   if((double)fSize/MAX != (int)fSize/(int)MAX){
     ACK_BUFFER_SIZE = ((int)fSize/(int)MAX) + 1;
   }  
   char **send_back_up;
    send_back_up = (char**)malloc(sizeof(char*)*ACK_BUFFER_SIZE);
   for(i = 0; i < ACK_BUFFER_SIZE; i++)send_back_up[i] = (char*)malloc(sizeof(char)*MAXLINE+1);
  
   ACK = (int*)malloc(sizeof(int)*ACK_BUFFER_SIZE);
   for(i = 0; i < ACK_BUFFER_SIZE; i++)ACK[i] = 0;

    while(1){
      ifconnect = fcntl(sockfd,F_GETFL,0);
      nrecv = recvfrom(sockfd,recv_buf,sizeof(recv_buf),0,pcliaddr,&clilen);
     printf("(%d,%s)\n",nrecv,recv_buf);
     if(atoi(recv_buf)!=0){
        index = atoi(recv_buf);
      }
     else if(atoi(recv_buf) == 0){ index = -1; }
 
     memset(recv_buf,0,sizeof(recv_buf));

      if(nrecv  < 0){
         // printf("nothing!\n");
          if(errno == EWOULDBLOCK){
             if(ifconnect){ 
             if(strlen(send_buf)!=0){
               for(i = 0; i < ACK_BUFFER_SIZE; i++){
                 if(ACK[i] == 0)sendto(sockfd,send_back_up[i],sizeof(send_back_up[i]),0,pcliaddr,clilen);break;
               }
            continue;    
             }
              }
         }
        else{
            printf("Recvfrom is error");
            return;
         } 

    }
    else if(nrecv == 0){printf("client out.\n");break;}
    else if(nrecv > 0){
     //printf("Get somthing!\n");
      if(index >= ACK_BUFFER_SIZE)goto END;
       
      else if(index != -1){
         if(ACK[index] == 0){ACK[index] = 1;}
         
         else if(index > 0 && ACK[index] == 1){
              sendto(sockfd,send_back_up[index],sizeof(send_back_up[index]),0,pcliaddr,clilen);          printf("already resend num:(%d) packet!\n",index);
         //continue;   
         }
      }
      
      if( (nread = fread(buff,sizeof(char),1490,from)) > 0){
        if(index == -1)index = 1;
       sprintf(seq_num,"%d",++seq);
       sprintf(send_buf,"%s%s",seq_num,buff);
       sendto(sockfd,send_buf,nread+strlen(seq_num),0,pcliaddr,clilen);
       memcpy(send_back_up[index],send_buf,sizeof(send_buf));
       printf("(%d,%ld)\n",nread,fSize);
       memset(send_buf,0,sizeof(send_buf));
      }
     else if(nread == 0){
         END:
         printf("Time to end!\n");
         memset(send_buf,0,sizeof(send_buf));
         if(strlen(send_buf)==0)printf("Time to end!\n");
        sendto(sockfd,send_buf,strlen(send_buf),0,pcliaddr,clilen);
        break;
      }
     //memset(recv_buf,0,sizeof(recv_buf));
    }
    else{
       printf("Client out");
       return;
    }

   }
  
  fclose(from);
}
