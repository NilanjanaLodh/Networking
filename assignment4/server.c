/*
    Simple udp server
*/
#include <stdio.h> //printf
#include <string.h> //memset

#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
 
#define BUFLEN 512  //Max length of buffer

int socklen; 
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
void bind_socket(int sockfd , int portno)
{
    struct sockaddr_in serv_addr;
    // zero out the structure
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(sockfd , (struct sockaddr*)&serv_addr, sizeof(serv_addr) ) == -1)
    {
        error("bind");
    }
}
int main(int argc, char **argv)
{
    struct sockaddr_in  cli_addr;
    if(argc<2)
    {
        printf("ERROR, no port provided\n");
        exit(1);
    }
    int portno = atoi(argv[1]);

    int sockfd, i,  recv_len;
    socklen = sizeof(cli_addr) ;
    char buf[BUFLEN];
     
    //create a UDP socket
    sockfd= get_sock_fd();
     
    //bind the socket to local interfaces
    bind_socket(sockfd, portno);
     


    //keep listening for data
    while(1)
    {
        printf("Waiting for data...");
        fflush(stdout);
        bzero(buf, BUFLEN);
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &cli_addr, &socklen)) == -1)
        {
            error("recvfrom()");
        }
         
        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        printf("Data: %s\n" , buf);
         
        //now reply the client with the same data
        if (sendto(sockfd, buf, recv_len, 0, (struct sockaddr*) &cli_addr, socklen) == -1)
        {
            error("sendto()");
        }
    }
 
    close(sockfd);
    return 0;
}