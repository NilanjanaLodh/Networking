/**single client , single server**/
/** they can continuosly chat with each other **/
// This way :

#include <bits/stdc++.h>
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

#include <arpa/inet.h>
#include <sqlite3.h>

#define OK 1
#define ERR 2
#define LOGOUT 3

using namespace std;

void error(char *msg)
{
	//this function called when a system call fails
	perror(msg);
	exit(1);
}


int get_sock_fd()
{
	int sfd =socket(AF_INET , SOCK_STREAM , 0);
	if(sfd<0)
		error("Error obtaining socket");

	else
		return sfd;
}

void bind_socket(int sockfd , int portno)
{
	//The IP adddress is implicitly the IP address of the host machine
	struct sockaddr_in serv_addr ;
	bzero((char*) &serv_addr , sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno); 
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    int b=bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if(b<0)
    	error("Binding Error");
}

void listen_socket(int sockfd)
{
	if (listen(sockfd, 5) < 0)
    {
        error("Listen error");
    }
}

int accept_connection(int sockfd)
{
	struct sockaddr_in  cli_addr;
	int clilen;

	clilen = sizeof(cli_addr);

	int newsockfd = accept(sockfd , (struct sockaddr*)&cli_addr, (socklen_t*)&clilen);
	if(newsockfd < 0)
		error("ERROR accepting connection");


	return newsockfd;
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

void print_ip_port(int sfd)
{
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];
	int port;
	len = sizeof addr;
	getpeername(sfd, (struct sockaddr*)&addr, &len);

	struct sockaddr_in *s = (struct sockaddr_in *)&addr;
	port = ntohs(s->sin_port);
	inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	printf("%s:%d\n",ipstr,port );
}
//#######################################################
// Real stuff starts here
sqlite3 *db;
bool isOpenDB = false;

int search_res_sock;
static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	printf("callback called\n");
	int i;
	search_res_sock=atoi(argv[0]);
	for(i=0; i<argc; i++){
	  printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}
void execute(const char *sql)
{
	
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK )
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Executed successfully.\n");
	}

}

bool connect_db(char dbname[100])
{
    if (sqlite3_open(dbname, &db) == SQLITE_OK)
    {
    	printf("Opened successfully\n");
        isOpenDB = true;
        return true;
    }         

    return false;
}

int insert_user(char username[256] , int sockfd)
{
	std::stringstream strm;
	strm<<"INSERT INTO USER (UNAME,SOCKFD) "  \
         "VALUES (\'"<<username<<"\',"<<sockfd<<" ); ";

	string s = strm.str();
	const char* p = s.c_str();
	execute(p);
	printf("Row inserted.\n");
}

void disonnect_db(sqlite3 *dbp)
{
	//printf("Lions den\n");
	//printf("%ud\n",dbp );
    if ( isOpenDB == true ) 
    {
        sqlite3_close(dbp);
    	printf("Disconnected !\n");
    }
}

void create_usertable()
{
	char *sql = "CREATE TABLE USER("  \
         "UNAME CHAR(256) PRIMARY KEY NOT NULL," \
         "SOCKFD INT NOT NULL)" ;
    /* Execute SQL statement */
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK )
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Table created successfully\n");
	}

}

struct user
{
	char uname[256];
	int sockfd;
};

struct user usertable[10];
int users;
int peer_sockfd[1000];
pthread_mutex_t mutex_lock;

int search_user(char name[256])
{
	// int i;
	// for(i=0;i<users;i++)
	// {
	// 	if(strcmp(usertable[i].uname,name)==0)
	// 		return i;
	// }
	// return -1;
	stringstream strm;
	strm<<"SELECT SOCKFD FROM USER WHERE UNAME=\'"<<name<<"\'";

	string s = strm.str();
	const char* p = s.c_str();
	search_res_sock=-1;
	execute(p);
	return search_res_sock;
}
int search_sock(int sfd)
{
	int i;
	for(i=0;i<users;i++)
	{
		if(usertable[i].sockfd == sfd)
			return i;
	}
	return -1;
}
void relay_msg(int sockfd)
{
	int psockfd = peer_sockfd[sockfd];
	//read from sockfd , and write to psockfd
	char buf[256];
	int d;
	while(1)
	{
		d = read_from_socket(sockfd, buf);
		if(d==1)
		{
			if(buf[0]==LOGOUT)
			{
				int uid = search_sock(sockfd);
				
				printf("%s logged out.\n", usertable[uid].uname);
				strcpy(usertable[uid].uname,"XXX");
				usertable[uid].sockfd = -1;
				close(sockfd);
				break;
				//exit(0);
			}
		}
		if(d>0)
		write_to_socket(psockfd, buf);
	}
}
void* authenticate_n_start(void *args)
{
	int sockfd = *(int*)args;
	char buf[256];
	int sfd;
	while(1)
	{
		read_from_socket(sockfd,buf);
		sfd = search_user(buf);
		
		if(sfd!=-1)
		{
			bzero(buf,256);
			buf[0]=ERR;
			write_to_socket(sockfd,buf);
		}
		else
		{
			pthread_mutex_lock(&mutex_lock);
			/**
			strcpy(usertable[users].uname,buf);
			usertable[users].sockfd = sockfd;
			
			**/
			//Use a database instead:
			users++;
			insert_user(buf,sockfd);
			pthread_mutex_unlock(&mutex_lock);
			
			printf("%s logged in.\n", buf);			
			bzero(buf,256);
			buf[0]=OK;
			write_to_socket(sockfd,buf);
			break;
		}
	}

	while(1)
	{
		read_from_socket(sockfd,buf);
		sfd = search_user(buf);
		
		if(sfd==-1)
		{
			bzero(buf,256);
			buf[0]=ERR;
			write_to_socket(sockfd,buf);
		}
		else
		{
			peer_sockfd[sockfd] = sfd;
			bzero(buf,256);
			buf[0]=OK;
			write_to_socket(sockfd,buf);
			break;
		}
	}
	printf("Client connected "); 
	print_ip_port(sockfd);
	printf("\n");

	relay_msg(sockfd);
	//exit(0);
	
}
void* exit_thread(void *args)
{
	sqlite3 *dbp = (sqlite3*)args;
	char buf[256];
	while(1)
	{
		scanf("%s",buf);
		if((strcmp(buf,"EXIT")==0)||(strcmp(buf,"exit")==0))
		{
			//printf("Before disconnect\n");
			disonnect_db(dbp);
			//printf("After disconnect\n");
			exit(0);
		}
	}
}

int main(int argc , char* argv[])
{
	pthread_t thread[50];
	pthread_t exit_thr;
	int t =0;
	if(argc<2)
	{
		printf("ERROR, no port provided\n");
		exit(1);
	}
	
	int portno = atoi(argv[1]);

	int sockfd= get_sock_fd();
	bind_socket(sockfd, portno);
	listen_socket(sockfd);
	printf("Waiting for clients to connect..\n");
	isOpenDB = false;
	connect_db("chatserver.db");
	pthread_create(&exit_thr , NULL , exit_thread , db);
	execute("drop table user");
	create_usertable();
	int newsockfd;

	users=0;
	int i;
	for(i=0;i<1000;i++)
		peer_sockfd[1000]=-1;
	pthread_mutex_init(&mutex_lock, NULL);

	while(1)
	{
		newsockfd=accept_connection(sockfd);
		pthread_create(&thread[t] , NULL , authenticate_n_start , (void*)&newsockfd);	
		t++;
	}

	

	printf("\n\n\n__________\nBye!\n");
	exit(0);

}
int test()
{
	isOpenDB = false;
	connect_db("chatserver.db");
	execute("drop table user");
	create_usertable();
	insert_user("One",1);
	insert_user("TwentyOne",21);
	insert_user("Three",3);
	
	printf(" Found %d\n", search_user("Three"));

}