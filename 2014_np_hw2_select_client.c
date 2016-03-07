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



void dg_cli(int,const struct sockaddr*,socklen_t);



int readable_timeo(int fd,int sec);



int main(int argc,char ** argv){



   int sockfd;

   struct sockaddr_in servaddr;

   

   if(argc != 3){

      printf("Enter <IP> <port number>\n");

      return 0;

   }

   

   if( (sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){

      printf("Socket creation is wrong\n");

      return 0;

   }



   memset(&servaddr,0,sizeof(servaddr));

   servaddr.sin_family = AF_INET;

   servaddr.sin_port = htons(atoi(argv[2]));

   servaddr.sin_addr.s_addr = inet_addr(argv[1]);

 

   dg_cli(sockfd,(const struct sockaddr*)&servaddr,sizeof(servaddr));



  return 0;

}





void dg_cli(int sockfd,const struct sockaddr* pservaddr,socklen_t servlen)

{

  FILE *Output;

   

    Output = fopen("Output2","wb");

     if(Output == NULL){

        printf("File Open Fail!\n");

        return;

      }



     int nread,nrecv;

     char sendbuffer[MAXLINE+1],recvbuffer[MAXLINE+1];

     int i;

     nread = 0;     

     int j;

     int seq[MAXROW];

     char ** back_up = (char**)malloc(sizeof(char*)*MAXROW);

    for(i = 0; i < MAXROW; i++){back_up[i] = (char*)malloc(sizeof(char)*MAXLINE+1);  } 

     for(i = 0; i < MAXROW; i++)seq[i] =0;

    sprintf(sendbuffer,"%s","Please send!");

     int ttt = sendto(sockfd,sendbuffer,strlen(sendbuffer)+1,0,

             pservaddr,servlen);           

     i = 1;

    while(1){

      

      if(readable_timeo(sockfd,2) == 0){        

                fprintf(stderr,"socket timeout\n");

          if(seq[i-1] == 0||seq[i] == 0){ 

              if(seq[i-1]==0)sprintf(sendbuffer,"%d",i-1);

           else{sprintf(sendbuffer,"%d",i);}

              sendto(sockfd,sendbuffer,strlen(sendbuffer),0,pservaddr,servlen);

              printf("num:(%s)packet please send again!\n",sendbuffer);            }

             // continue;

         }

         else{

      

            nrecv = recvfrom(sockfd,recvbuffer,MAXLINE+1,0,NULL,NULL);  

          //  printf("%d\n",recv);

         if(nrecv < 0){

               if(seq[i-1] == 0||seq[i] == 0){

              if(seq[i-1]==0)sprintf(sendbuffer,"%d",i-1);

           else{sprintf(sendbuffer,"%d",i);}

              sendto(sockfd,sendbuffer,strlen(sendbuffer),0,pservaddr,servlen);

              printf("num:(%s)packet please send again!\n",sendbuffer);            }

               printf("recvfrom error!\n");

               break;

         }

        else if(nrecv == 0){

              memset(sendbuffer,0,sizeof(sendbuffer));

              sendto(sockfd,sendbuffer,strlen(sendbuffer),0,pservaddr,servlen);

              int max_seq;

              for(j = 0; j <= MAXROW; j++){

                if(seq[j])max_seq = j;

                else if(seq[j] == 0 && seq[j+1]==1)max_seq = j+1;

                else break;

              }

              for(i = 0; i <= max_seq; i++){

               printf("%s\n",back_up[i]);

               fwrite(back_up[i],sizeof(char),strlen(back_up[i]),Output);

               fflush(Output);

              }

              

              printf("\n\nfinish! normal close.\n\n");

              break;}

      else  if(nrecv > 0){

           //  printf("%d ",nrecv);

            

             if(seq[atoi(recvbuffer)] == 1)continue;

              seq[atoi(recvbuffer)] = 1;

             if(strlen(recvbuffer) == 0)continue;

              char seq_num[10];

          sprintf(seq_num,"%d",atoi(recvbuffer));

          memcpy(back_up[atoi(recvbuffer)],recvbuffer+strlen(seq_num),sizeof(char)*nrecv-strlen(seq_num));

          back_up[atoi(recvbuffer)][strlen(back_up[atoi(recvbuffer)])] = 0;



              memset(sendbuffer,0,sizeof(sendbuffer));

              sprintf(sendbuffer,"%d\n",i);

              printf("Please send : %s\n",sendbuffer);

              i++;

           sendto(sockfd,sendbuffer,strlen(sendbuffer),0,pservaddr,servlen);     

             //  printf("%s\n",sendbuffer);

               memset(recvbuffer,0,sizeof(recvbuffer));

               memset(sendbuffer,0,sizeof(sendbuffer));

            }       

        }

     }

   fclose(Output);



 return;

 }



int readable_timeo(int fd,int sec)

{

  fd_set rset;

  struct timeval tv;



  FD_ZERO(&rset);

  FD_SET(fd,&rset);

  

  tv.tv_sec = 0;

  tv.tv_usec = 20;



  return (select(fd+1,&rset,NULL,NULL,&tv));

}
