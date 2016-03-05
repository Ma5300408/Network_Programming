#include<stdio.h>

#include<stdlib.h>

#include<math.h>

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

void sig_alrm(int signo){

  return;

}



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

    for(i = 0; i < argc;i++){

                       // printf("%s\n",argv[i]);

      memcpy(argv_gbl[i],argv[i],strlen(argv[i])*4);

                       // printf("%s\n",argv_gbl[i]);   

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

     int nread,nrecv;

     char send_buf[MAXLINE+1],recv_buf[MAXLINE],buf[MAXLINE];

     struct timeval tv;

     char seq_num[10];

     int count;

     long fSize;

     double MAX = MAXLINE;

     int* ACK;

     fseek(from,0,SEEK_END);

     fSize = ftell(from);     

     rewind(from);

     nread = -1;

     count = 0;

     signal(SIGALRM,sig_alrm);

     siginterrupt(SIGALRM,1);

     int ACK_BUFFER_SIZE;

     int index;

     int seq;

     seq = 0;

     if((double)fSize/MAX !=(int)fSize/(int)MAX){

         ACK_BUFFER_SIZE= ((int)fSize/(int)MAX) + 1;

     }

      char **send_back_up;

    send_back_up = (char**)malloc(sizeof(char*)*ACK_BUFFER_SIZE);

   for(i = 0; i < ACK_BUFFER_SIZE; i++)send_back_up[i] = (char*)malloc(sizeof(char)*MAXLINE+1);



     ACK = (int*)malloc(sizeof(int)*ACK_BUFFER_SIZE);

     for(i = 0; i < ACK_BUFFER_SIZE; i++)ACK[i] = 0;

     printf("%d\n",ACK_BUFFER_SIZE);

    while(1){

      ualarm(20,5);

      // alarm(2);

      nrecv = recvfrom(sockfd,recv_buf,sizeof(recv_buf),0,pcliaddr,&clilen);

        if(atoi(recv_buf)!=0){

           index = atoi(recv_buf);

          }

         else if(atoi(recv_buf)==0) { index = -1; }

         memset(recv_buf,0,sizeof(recv_buf));

         if( nrecv  < 0 ){

       		 if(errno == EINTR){

            	      if(strlen(send_buf)!=0){

            		   for(i = 0; i < ACK_BUFFER_SIZE; i++){

             			if(ACK[i]==0){sendto(sockfd,send_back_up[i],sizeof(send_back_up[i]),0,pcliaddr,clilen);   break;}

             	

                     }

                        

           	continue;}

             	    printf("Time out: \n"); 

       	 	}

  	    else{

        	   printf("Recvfrom error!\n");

        	   break;

      	        }

     

    	 }

    else if(nrecv == 0){printf("client out.\n");break;}

    else if(nrecv > 0){

     

      if(index >= ACK_BUFFER_SIZE+3)goto END;     

     

      else if(index!=-1){

             if(ACK[index] == 0){ ACK[index]=1;}



             else if(index > 0 && ACK[index]== 1){

                    sendto(sockfd,send_back_up[index],sizeof(send_back_up[index]),0,pcliaddr,clilen);                      printf("already resend %d packet!\n",index); 

                    }

          }

        if( (nread = fread(buf,sizeof(char),1490,from)) > 0){

                if(index == -1)index = 1;

       sprintf(seq_num,"%d",++seq);

       sprintf(send_buf,"%s%s",seq_num,buf);

       sendto(sockfd,send_buf,nread+strlen(seq_num),0,pcliaddr,clilen);

       memcpy(send_back_up[index],send_buf+strlen(seq_num),sizeof(char)*(nread+strlen(seq_num)));

      memset(send_buf,0,sizeof(send_buf));

           }

  

    else if (nread == 0){

         /*for(i = 0; i < ACK_BUFFER_SIZE; i++){

            if(ACK[i] == 0){

             sendto(sockfd,send_back_up[i],strlen(send_back_up[i]),0,pcliaddr,clilen);

            }

         }

       nread = 0;*/

       END:

       printf("ready to over!\n");

       memset(send_buf,0,sizeof(send_buf));

       sendto(sockfd,send_buf,strlen(send_buf),0,pcliaddr,clilen);

     // memset(ACK,0,sizeof(ACK)*ACK_BUFFER_SIZE);

    //  free(ACK);

     // goto END; 

       break;

     }

   }

   

  }

 

  fclose(from);

}
