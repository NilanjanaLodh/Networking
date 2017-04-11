/** now clients will talk to each other via the server **/
#include <stdio.h>
#include <string.h> // strlen
#include <stdlib.h> // strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h> // write
#include <pthread.h> // for threading, link with lpthread

#include <sys/types.h> //data type definitions
#include <netinet/in.h> //constants for internet domain addresses
#include <netdb.h> // <-- this one's new! (For hostnet)

#define OK 1
#define ERR 2
#define LOGOUT 3

void error(char *msg)
{
	//this function called when a system call fails
	perror(msg);
	exit(1);
}
int get_sock_fd()
{

	int sfd =socket(AF_INET , SOCK_STREAM , 0); 

	/**
	socket()  creates  an  endpoint  for  communication  and returns a file
       descriptor that refers to that endpoint.
	**/
	//AF_INET : IPv4
	//SOCK_STREAM : TCP

	if(sfd<0)
		error("Error obtaining socket");

	else
		return sfd;
}

void connect_socket(int sockfd,char *hname, int portno)
{
	struct sockaddr_in serv_addr;
	struct hostent *server;
	server = gethostbyname(hname);
	if(server == NULL)
		error("ERROR , no such host\n");

	bzero((char *) &serv_addr , sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr,(char *)&serv_addr.sin_addr.s_addr , server->h_length);
	serv_addr.sin_port = htons(portno);
	if(connect(sockfd ,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");
}

void write_to_socket(int sockfd ,char buf[256])
{
	int n = write(sockfd, buf , strlen(buf));

	if(n<0)
		error("ERROR writing to socket");
}

int read_from_socket(int sockfd , char buf[256])
{

	bzero(buf, 256);
	int n = read(sockfd,buf,255);

	if(n<0)
		error("ERROR reading from socket");
	return n;

}

void* recieve(void *args)
{
	char buf[256];
	int sockfd = (int)args;
	int d;
	while(1)
	{
		d=read_from_socket(sockfd, buf);
		
		if(d>0)
		printf("			<< %s\n",buf);
	}
	return NULL;
}

char user_name[256] , peer_name[256];
void authenticate(int sockfd)
{
	char buf[256];
	int err=0;
	do
	{
		printf("Username : ");
		scanf("%s",user_name);
		write_to_socket(sockfd, user_name);
		read_from_socket(sockfd, buf);
		if(buf[0]==ERR)
		{
			printf("Username already taken, try again.\n");
			err=1;
		}
		else
			err=0;
	}while(err==1);
	
	do
	{
		printf("Peer's username : ");
		scanf("%s",peer_name);
		write_to_socket(sockfd, peer_name);
		read_from_socket(sockfd, buf);
		if(buf[0]==ERR)
		{
			printf("Peer not connected, try again.\n");
			err=1;
		}
		else
			err=0;
	}while(err==1);
	

}
int main(int argc , char* argv[])
{
	pthread_t rcv_thread;

	if(argc < 3)
	{
		printf("Usage %s server_hostname server_port\n", argv[0]);
		exit(0);
	}

	int portno=  atoi(argv[2]);
	int sockfd= get_sock_fd();
	

	connect_socket(sockfd, argv[1] , portno);
	authenticate(sockfd);
	printf("Connection established.\n___________________________\n");

	pthread_create(&rcv_thread , NULL , recieve , (void*)sockfd);	

	char buf[256];
	
	while(1)
	{
		bzero(buf,256);	
		fgets(buf, 255 , stdin);
		if(strcmp("LOGOUT\n",buf)==0)
		{
			printf("Attempting to logout\n");
			bzero(buf,256);
			buf[0]=LOGOUT;
			write_to_socket(sockfd,buf);
			break;
		}

		if(strlen(buf)>0)
		{
			write_to_socket(sockfd,buf);
		}

	}
	close(sockfd);
	printf("\n\n\n__________\nLogged out!\n");
	exit(0);

}