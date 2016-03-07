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

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MAXLINE 1024

void str_cli(FILE *fp, int sockfd);

int main(int argc,char ** argv)
{
   int sockfd;
   struct sockaddr_in servaddr;
   
   if(argc != 3){
     printf("usage: <IP> <Port number>\n");
     return 0;
   }      
  
   sockfd = socket(AF_INET,SOCK_STREAM,0);  
   memset(&servaddr,0,sizeof(servaddr));
   
   servaddr.sin_family = AF_INET;
 servaddr.sin_port = htons(atoi(argv[2]));
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);   
/*   printf("before max\n"); 
   printf("%d\n",MAX(100,1));
   printf("%d\n",MAX(1,100));
   printf("after max\n");*/
   connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
   str_cli(stdin,sockfd);
   exit(0);
}

void str_cli(FILE *fp, int sockfd)
{
     int maxfdp1;
     fd_set rset;
     char sendline[MAXLINE],recvline[MAXLINE];
     char buff[MAXLINE];
     int nrecv;
     int tmp; 
     
    int stdineof = 0;
    FD_ZERO(&rset);
        memset(sendline,0,sizeof(sendline));
        memset(recvline,0,sizeof(recvline)); 
    for(;;){
      
      
       if(stdineof == 0)FD_SET(fileno(fp),&rset);
        FD_ZERO(&rset);
        FD_SET(fileno(fp),&rset);
        FD_SET(sockfd,&rset);
        maxfdp1 = MAX(fileno(fp),sockfd)+1;
        select(maxfdp1,&rset,NULL,NULL,NULL);
      
       if(FD_ISSET(sockfd,&rset)){ /* socket is readable */
             // memset(recvline,0,sizeof(recvline));
           if( (tmp=read(sockfd,recvline,MAXLINE)) == 0){
              if(stdineof == 1)return;
              printf("str_cli: server terminated prematurely");
               return;
              }
          // printf("recv_len: %d\n",tmp);
           fputs(recvline,stdout);
           memset(recvline,0,sizeof(recvline));
        }
       
        if(FD_ISSET(fileno(fp),&rset)){ /*input is readable*/
            if(fgets(sendline,MAXLINE,fp) == NULL){
                stdineof = 1;
                shutdown(sockfd,SHUT_WR); /* send FIN */
                FD_CLR(fileno(fp),&rset);
                continue;
                return;
             }
            write(sockfd,sendline,strlen(sendline));
           // printf("send_len: %d\n",strlen(sendline));
            memset(sendline,0,sizeof(sendline));
        }      

    }

}
