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



int readable_timeo(int fd,int sec);





char **argv_gbl;



int main(int argc,char ** argv)

{

    int sockfd;

    struct sockaddr_in cliaddr,servaddr;

    if(argc != 3){

       printf("Enter <port number> <file name>\n");

       return 0;

    }

   if( (sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0 ){

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

  return 0;

}



void dg_echo(int sockfd,struct sockaddr * pcliaddr,socklen_t clilen)

{



   FILE *from;   



    from = fopen(argv_gbl[2],"rb");

    

    if(from == NULL){

       printf("File not exist");

       return;

    }

    int count;  

    int  i;

    int nread,nrecv;

    char send_buf[MAXLINE+1],recv_buf[MAXLINE],buff[MAXLINE]; 

    double MAX = MAXLINE;

    long fSize;

    int* ACK; 

    char seq_num[10];

     nrecv = -1;

     nread = 0;

     count = 0;

    int ACK_BUFFER_SIZE;

    int index;

    int seq;

    seq = 0;

    fseek(from,0,SEEK_END);

    fSize = ftell(from);

    rewind(from);



    if((double)fSize/MAX != (int)fSize/(int)MAX){

       ACK_BUFFER_SIZE = ((int)fSize/(int)MAX) + 1;

    }

    

    char **send_back_up;

    send_back_up = (char**)malloc(sizeof(char*)*ACK_BUFFER_SIZE);

   for(i = 0; i < ACK_BUFFER_SIZE; i++){send_back_up[i] = (char*)malloc(sizeof(char)*MAXLINE+1);}

   

    ACK = (int*)malloc(sizeof(int)*ACK_BUFFER_SIZE);

   for(i = 0; i < ACK_BUFFER_SIZE; i++)ACK[i] = 0;    



   while(1){

      

       

       if(readable_timeo(sockfd,2)== 0){

        fprintf(stderr,"socket timeout\n");

        if(strlen(send_buf)!=0){

         for(i = 0; i < ACK_BUFFER_SIZE; i++){

              if(ACK[i] == 0){sendto(sockfd,send_back_up[i],sizeof(send_back_up[i]),0,pcliaddr,clilen); printf("Hey! I resend %d for you.!\n",i); break;}

                              }

                    continue;}

     }else{

         

         nrecv = recvfrom(sockfd,recv_buf,sizeof(recv_buf),0,pcliaddr,&clilen);

            

         if(atoi(recv_buf)!=0){

              index = atoi(recv_buf);

         }

        else if(atoi(recv_buf) == 0){index = -1; }

        printf("(%d,%s)\n",nrecv,recv_buf);

        memset(recv_buf,0,sizeof(recv_buf));



        // printf("(%d,%s)",nrecv,recv_buf);      

        if(nrecv < 0){

           printf("recvfrom error!\n");

           return;

         }

        else if(nrecv == 0){printf("client out.\n");break;}

        else if(nrecv > 0){

          

            if(index >= ACK_BUFFER_SIZE )goto END;

           

           else if(index != -1){

                if(ACK[index] == 0){ACK[index] = 1;}

               

               else if(index > 0 && ACK[index] == 1){

                 sendto(sockfd,send_back_up[index],sizeof(send_back_up[index]),0,pcliaddr,clilen); printf("already resend num:(%d) packet!\n",index);

              }

           }





        if(( nread = fread(buff,sizeof(char),sizeof(buff),from) ) > 0){

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

             printf("Send All Article.\n");

             memset(send_buf,0,sizeof(send_buf));

             sendto(sockfd,send_buf,strlen(send_buf),0,pcliaddr,clilen);

             break;

        }

      }

   }

    

     

     memset(recv_buf,0,sizeof(recv_buf));



   }



  fclose(from);

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

