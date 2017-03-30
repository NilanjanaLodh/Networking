/*
    Simple udp client
*/
#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
 

#define BUFLEN 512  //Max length of buffer
#define SERVER "localhost"
 
void error(char *s)
{
    perror(s);
    exit(1);
}
 
int get_sock_fd()
{
    int sockfd;
    if ((sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        error("socket");
    }
    return sockfd;
}
void fill_struct(char* server_hostname, int portno ,struct sockaddr_in *serv_addr , int addrlen)
{
    memset((char *)serv_addr, 0, addrlen);
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(portno);
     
    if (inet_aton(server_hostname , &(serv_addr->sin_addr)) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }    
}
int main(int argc , char ** argv)
{
    if(argc < 3)
    {
        printf("Usage %s server_hostname server_port\n", argv[0]);
        exit(0);
    }

    int portno=  atoi(argv[2]);
    char *server_hostname=argv[1];

    struct sockaddr_in serv_addr;
    int sockfd, i, slen=sizeof(serv_addr);
    char buf[BUFLEN];
    char message[BUFLEN];
 
    sockfd = get_sock_fd();

    fill_struct(server_hostname, portno , &serv_addr , slen);
    while(1)
    {
        bzero(buf, BUFLEN);
        bzero(message, BUFLEN);
        printf("Enter message : ");
        fgets(message,BUFLEN,stdin);
         
        //send the message
        if (sendto(sockfd, message, strlen(message) , 0 , (struct sockaddr *) &serv_addr, slen)==-1)
        {
            error("sendto()");
        }
         
        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        memset(buf,'\0', BUFLEN);
        //try to receive some data, this is a blocking call
        if (recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &serv_addr, &slen) == -1)
        {
            error("recvfrom()");
        }
         
        puts(buf);
    }
 
    close(sockfd);
    return 0;
}