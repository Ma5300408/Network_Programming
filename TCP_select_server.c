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



#define MAXLINE 1024

#define LISTENQ 10 //BACKLOG = 10





int main(int argc,char **argv)

{

  int i,maxi,maxfd,listenfd,connfd,sockfd;

  int nready;

  int client[FD_SETSIZE];  // num[BACKLOG]

  ssize_t n;

  fd_set rset,allset;

  char line[LISTENQ][MAXLINE];

 char buffer[MAXLINE];

  socklen_t clilen;  // sin_size

  struct sockaddr_in cliaddr,servaddr;

  char head[MAXLINE];

  char welcome[35];

  char client_name[LISTENQ][13];

  listenfd = socket(AF_INET,SOCK_STREAM,0);



  memset(&servaddr,0,sizeof(servaddr));

  servaddr.sin_family = AF_INET;

  servaddr.sin_port = htons(atoi(argv[2]));

  servaddr.sin_addr.s_addr = inet_addr(argv[1]);

  

  bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));



  listen(listenfd,LISTENQ);



  maxfd = listenfd; /* initialize  maxsock = sock_fd*/ 

  maxi = 0; /* index into client[] array */  

  for(i = 0; i < FD_SETSIZE; i++)client[i] = -1;

 /* -1 indicates available entry*/ 

    FD_ZERO(&allset);

  FD_SET(listenfd,&allset);

  for(;;){

       rset = allset; 

    //for(i = 0; i < FD_SETSIZE; i++)client[i] = -1;

 /* -1 indicates available entry*/

  

    if( (nready = select(maxfd+1,&rset,NULL,NULL,NULL)) == -1){

    	perror("Select() error.\n");

    	exit(EXIT_FAILURE);

    }

    

     // check whether a new connection comes

     if(FD_ISSET(listenfd,&rset)){

        

		clilen = sizeof(cliaddr);  

		if( (connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen) ) <= 0 ){

	       perror("accpet"); 	

	       continue;    	

	    }

       //#ifdef NOTDEF

       //#endif

       for(i = 0; i < FD_SETSIZE; i++){ //add to fd queue

             if(client[i] < 0){

                 client[i] = connfd; 

                 

        sprintf(client_name[i],"anonymous%d",i);

	 printf("new client : %s port %d\n",

               	 inet_ntop(AF_INET,&cliaddr.sin_addr,buffer,clilen),

                 ntohs(cliaddr.sin_port));

		// write(client[i],"connect\n",8);

             // sprintf(client_name[i],"anonymous%d",i);

              sprintf(head,"[Server] Hello, anonymous! From: <%s>/<%d>\n",buffer,ntohs(cliaddr.sin_port));	

        

        if(i > maxi){

        	maxi = i;

        	printf("Now Client number : %d\n",maxi+1);

            strcpy(welcome,"[Server] Someone is coming!\n");

            for(i = 0; i < maxi; i++)write(client[i],welcome,strlen(welcome));

        }

  

               	write(client[i],head,strlen(head));



		break;

              }

       }

         

        if(i == FD_SETSIZE)printf("too many clients");

        

        FD_SET(connfd,&allset); 

        if(connfd > maxfd)maxfd = connfd; 

        

        if(--nready <= 0)continue;

      }

    

    

    

    



    for(i = 0; i <= maxi; i++){

 

      if((sockfd = client[i]) < 0)continue;

  

      if(FD_ISSET(sockfd,&rset)){

          if( (n = read(sockfd,line[i],MAXLINE)) == 0){

             close(sockfd);

             FD_CLR(sockfd,&allset);

             int close_user_id = i;

             char name[MAXLINE];

      sprintf(name,"[Server] %s[%d] is offline\n",client_name[i],close_user_id);

             memset(client_name[i],0,sizeof(client_name[close_user_id]));

             for(i = 0; i <= maxi; i++){

               if(i!=close_user_id)write(client[i],name,strlen(name));}

             client[close_user_id] = -1;}

          else{

                   int client_self = sockfd;

                   if(n==1)continue;

                else if(n==4 && (strstr(line[i],"who")!=0)){

                        for(i = 0; i<=maxi;i++)if(client[i]!=0){

                       int sent_sock = client[i];

                   socklen_t sock_len;  

                struct sockaddr_in sock_tmp;



          getsockname(sent_sock,(struct sockaddr *)&sock_tmp,&sock_len);

          memset(buffer,0,sizeof(buffer));

          snprintf(buffer,sizeof(buffer),"[server] %s Client_IP/Port: %s/%d\n",client_name[i],

inet_ntoa(sock_tmp.sin_addr),ntohs(sock_tmp.sin_port));

                      write(client_self,buffer,strlen(buffer));

                       }

                    }

                   

                 else if(strstr(line[i],"tell")!=0){

                     char *pch;

                     char from_name[MAXLINE];

                     char name[13];

                     char msg[MAXLINE];

                     

                     pch = strtok(client_name[i],"\n");

                     sprintf(from_name,"[Server] %s",client_name[i]);

                     pch = strtok(line[i]," ");

                     pch = strtok(NULL," ");                      

                     memcpy(name,pch,sizeof(name));

                     name[strlen(name)+1] ='\0';

                     pch = strtok(NULL,"\n");

                     if(pch==NULL){

                       printf("No message passing! Error Format!\n");

                       break;

                     }

                    memcpy(msg,pch,sizeof(msg));

                  //  printf("after msg.\n");

                      char *numm = NULL;

                      for(i=0; i<=maxi; i++){

                      // printf("%s vs %s\n",name,client_name[i]);

                      // printf("%d vs %d\n",strlen(name),strlen(client_name[i]));

                     if( (numm = strstr(name,client_name[i]))!=NULL){

                       write(client_self,"[Server] SUCCESS: Your message has been sent.",46);

                      strcpy(from_name+strlen(from_name),":");

                        printf("after\n");

                        strcat(from_name,msg);

                        printf("after strcat.\n"); 

                        write(client[i],from_name,strlen(from_name));

                        break;

                     }

                  }

               if(numm == NULL){write(client_self,"[Server] ERROR The receive doesn't exist.\n",43);}



                 }

                 else if( (strstr(line[i],"name")) != 0){

                        char *pch;

                        char new_name[13];

                        pch = strtok(line[i]," ");

                        pch = strtok(NULL,"\n");

                        int j;

                       for(j = 0; j <= maxi; j++){



             if( strstr(pch,client_name[j])!=NULL ){

                sprintf(buffer,"%s","Repeat someone name! neglect this command.\n");       

                 write(client[i],buffer,strlen(buffer));

                 goto L1; 

                 }

                    }

                              

                   if(pch!=NULL){

           printf("[Server] %s is now known as %s ",client_name[i],pch);

                         memset(client_name[i],0,sizeof(client_name[i]));

                         strcpy(client_name[i],pch);

                         memset(buffer,0,sizeof(buffer));

                         sprintf(buffer,"[Server] You're now known as %s\n",client_name[i]);

                         write(client[i],buffer,strlen(buffer));

                       }

                }

                 else if( (strstr(line[i],"yell"))!=0 ){

                         memset(head,0,sizeof(head)); 

                 	 memset(buffer,0,sizeof(buffer));

                 	 sprintf(head,"[Server] %s:%s",client_name[i],line[i]);

                 	// strcat(head,line[i]); 

                 	 memset(line[i],0,sizeof(line[i]));

                 	 printf("%s",head);

                 	 memcpy(buffer,head,sizeof(buffer));

                 	 for(i = 0; i<=maxi;i++){

                   	 if(client_self != i){

                     	 write(client[i],buffer,strlen(buffer));

                   	 }                  

                       }

                      memset(buffer,0,sizeof(buffer));

                    }

              

                else{

                        sprintf(buffer,"[Server] Error command %s",line[i]);

                        write(client_self,buffer,strlen(buffer));

                    }

          }

          L1:

          

          if(--nready <= 0)break;    

         

       }      

    } 

  }

}

