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
#define MAXROW 4000

int last;

void dg_cli(int,const struct sockaddr*,socklen_t);

int main(int argc,char ** argv)
{
   int sockfd;
   struct sockaddr_in servaddr;
   //socklen_t servlen;
   if(argc != 3){
     printf("Enter <IP> <port number>\n");
     return 0;
   }
 
  if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
    perror("Socket creation is wrong\n");
    return 0;
   }
   //printf("%d\n",sockfd);
   memset(&servaddr,0,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(atoi(argv[2]));
   servaddr.sin_addr.s_addr = inet_addr(argv[1]);
  
   dg_cli(sockfd,(const struct sockaddr*)&servaddr,sizeof(servaddr));
  
   return 0;
}

void dg_cli(int sockfd,const struct sockaddr* pservaddr,socklen_t servlen)
{
   
    int n;
    char sendline[MAXLINE],recvline[MAXLINE];
    struct timeval tv;
    int i; 
    int j;
    int seq[MAXROW];
    char ** back_up;
    //printf("hihihhi!\n");
    back_up = (char**)malloc(sizeof(char*)*MAXROW);
    for(i = 0; i < MAXROW; i++){back_up[i] = (char*)malloc(sizeof(char)*(MAXLINE+1));  }
    for(i = 0; i < MAXROW; i++)seq[i] = 0;  
   
    FILE * Output;
    Output = fopen("Output1","wb");

    if(!Output){
	printf("fopen error");
	return;
    }    
        
    tv.tv_sec = 0;
    tv.tv_usec = 20;
     int yy;
    if( (yy = setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv)) ) <0 ){printf("%d\n",yy);
        perror("setsockopt fail!\n");
        return;
      }

    i = 1;
    //printf("Yes!\n");
    sprintf(sendline,"%s","Please send!");
    sendto(sockfd,sendline,strlen(sendline),0,pservaddr,servlen);
    memset(sendline,0,sizeof(sendline));
  
   
    while(1)
    {
       
       n = recvfrom(sockfd,recvline,MAXLINE,0,NULL,NULL);
     if(n < 0){
         if(errno == EWOULDBLOCK){
             fprintf(stderr," socket timeout\n");
             if(seq[i-1]==0)sprintf(sendline,"%d",i-1);
             else{sprintf(sendline,"%d",i);}
              sendto(sockfd,sendline,strlen(sendline),0,pservaddr,servlen);
              printf("num:(%s)packet send again!\n",sendline);
              continue;

         }else{ perror("recvfrom error"); break;}

     }
     else if( n == 0 ){
        last = i;
        memset(sendline,0,sizeof(sendline));
        sendto(sockfd,sendline,strlen(sendline),0,pservaddr,servlen);
        int max_seq;
        for(j = 0; j <= MAXROW; j++){
           if(seq[j]!=0)max_seq = j;
          // else if(seq[j]=0 && seq[j+1]!=0)max_seq = j+1;
           else {};
        }
         printf("%d\n",last);
        for(i = 0; i < last; i++){
         // printf("%s\n",back_up[i]);
         fwrite(back_up[i],sizeof(char),strlen(back_up[i]),Output);
         fflush(Output);
         // printf("%s\n",back_up[i]);
        }        

        printf("\n\n\nfinish! normal close!\n\n\n");
        break;}
     
     else if(n > 0){
          printf("len:%d\n",n);
          if(seq[atoi(recvline)]==1)continue;
          seq[atoi(recvline)] = 1; 
          if(strlen(recvline)==0)continue;
          char seq_num[10];
          sprintf(seq_num,"%d",atoi(recvline));
          memcpy(back_up[atoi(recvline)],recvline+strlen(seq_num),sizeof(char)*(n-strlen(seq_num)));
         // back_up[atoi(recvline)][strlen(back_up[atoi(recvline)])] = 0;
          //fwrite(recvline+1,sizeof(char),n-1,Output);
          
          sprintf(sendline,"%d",i);
          printf("Please send %s:\n",sendline);
          printf("%d\n",atoi(recvline));
          i++;
    	 sendto(sockfd,sendline,strlen(sendline),0,pservaddr,servlen);
         printf("%s\n",sendline);
         memset(recvline,0,sizeof(recvline));
         memset(sendline,0,sizeof(sendline));
      }
    }


fclose(Output);

}
