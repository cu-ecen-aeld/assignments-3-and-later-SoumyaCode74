#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <syslog.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>

#define BUFFER_SIZE	(1024)


int socket_fd,accept_return; //Socket fd and client connection
int status; //Daemon return status

//Structure for bind and accept
struct addrinfo hints;
struct addrinfo *servinfo;
struct sockaddr_in client_addr;

char *store_data=NULL; //To store data from receiver


//Signal handler
void signal_handler(int sig)
{
	if(sig==SIGINT)
	{
		syslog(LOG_INFO,"Caught SIGINT, exiting");
	}
	else if(sig==SIGTERM)
	{
		syslog(LOG_INFO,"Caught SIGTERM, exiting");
	}
	
	//Delete the logging file
	if(unlink("/var/tmp/aesdsocketdata") == -1){
		syslog(LOG_ERR, "logger could not be deleted!");
		exit(1);
	}
	//Close client connection
	close(accept_return);
	//Close socket
	close(socket_fd);	
	exit(0); 
}


int main(int argc, char *argv[])
{
	int getaddr,bind_status,listen_status;
	ssize_t rec_status=1,write_status;
	socklen_t size=sizeof(struct sockaddr); 
	char buff[BUFFER_SIZE]; //Set up buffer of 1 KB
	int fd_status,send_status;
	int total_bytes=0,packet_bytes_total=0;
	int write_flag=1;
	int i; 
	
	openlog(NULL,LOG_PID, LOG_USER | LOG_PERROR | LOG_CONS); //Initialize system logger
	
	
	//To start a daemon process
	if((argc>1) && strcmp(argv[1],"-d")==0)
	{
		if(daemon(0,0)==-1)
		{
			perror("Daemon not started!");
			syslog(LOG_ERR, "Couldn't enter daemon mode");
			exit(1);
		}
	}
	
	//To check if a signal is received
	if(signal(SIGINT,signal_handler)==SIG_ERR)
	{
		syslog(LOG_ERR,"SIGINT failed");
		exit(1);
	}
	if(signal(SIGTERM,signal_handler)==SIG_ERR)
	{
		syslog(LOG_ERR,"SIGTERM failed");
		exit(1);
	}
	
	socket_fd=socket(PF_INET, SOCK_STREAM, 0);
	if(socket_fd==-1)
	{
		perror("Socket could not be created!");
		syslog(LOG_ERR, "Socket could not be created");
		exit(1);
	}
	
	hints.ai_flags=AI_PASSIVE;
	getaddr=getaddrinfo(NULL,"9000",&hints,&servinfo);
	if(getaddr !=0)
	{
		perror("Address could not be fetched");
		syslog(LOG_ERR, "Couldn't get the server's address");
		exit(1);
	}
	bind_status=bind(socket_fd,servinfo->ai_addr,sizeof(struct sockaddr));
	if(bind_status==-1)
	{
		printf("Binding failed\n");
		perror("Binding error");
		freeaddrinfo(servinfo);
		syslog(LOG_ERR, "Binding failed");
		exit(1);
	}
	
	freeaddrinfo(servinfo);
	
	fd_status=open("/var/tmp/aesdsocketdata", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO); //To create a file and give permissions to all
	if (fd_status==-1)
	{
		perror("The file could not be created or found");
		syslog(LOG_ERR, "The file could not be created or found");
		exit(1);
	}
	close(fd_status);
	
	while(1)
	{
	
	listen_status=listen(socket_fd,100);
	if(listen_status==-1)
	{
		syslog(LOG_ERR, "Listening to the connections failed");
		exit(1);
	}
	
	accept_return=accept(socket_fd,(struct sockaddr *)&client_addr,&size);
	if(accept_return==-1)
	{
		perror("Connection could not be accepted");
		syslog(LOG_ERR, "Connection could not be accepted");
		exit(1);
	}
	else
	{
		syslog(LOG_INFO,"Accepted connection from %s",inet_ntoa(client_addr.sin_addr));
		printf("Accepted connection from %s\n",inet_ntoa(client_addr.sin_addr));
	}
	
	
	store_data = (char*)malloc(BUFFER_SIZE);
	if(store_data==NULL)
	{
		perror("Memory could not be allocated");
		syslog(LOG_ERR, "Memory couldn't be allocated");
		exit(1);
	}
	
	memset(store_data,0,BUFFER_SIZE);
	
	write_flag=0;
	
	while(!write_flag) // Start infinite loop to keep accepting incoming messages until signal comes
	{
	
	rec_status=recv(accept_return,buff,BUFFER_SIZE,0);
	if(rec_status==-1)
	{
		syslog(LOG_ERR, "Error in reception of data packets from client");
		exit(1);
	}
	
	
	for(i=0;i<BUFFER_SIZE;i++)
	{
		if(buff[i]=='\n')
		{
			i++;
			write_flag=1;
			break;
		}	
		
	}
	total_bytes+=i;
	printf("Total bytes received till now: %d\n", total_bytes);
	
	store_data=(char *)realloc(store_data,total_bytes);
	if(store_data==NULL)
	{
		perror("Memory could not be allocated");
		syslog(LOG_ERR, "Reallocation of memory failed");
		exit(1);
	}
	memcpy(store_data+total_bytes-i,buff,i);
	memset(buff,0,BUFFER_SIZE);
	}
	
	fd_status = open("/var/tmp/aesdsocketdata", O_APPEND | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	if(fd_status==-1)
	{
		perror("Couldn't open file");
		syslog(LOG_ERR, "Couldn't open file");
		exit(1);
	}
		
	
	write_status= write(fd_status,store_data,total_bytes);
	if(write_status!=total_bytes)
	{
		perror("Could not write all the bytes to the file");
		syslog(LOG_ERR, "Could not write all the bytes to the file");
		exit(1);
	}
	close(fd_status);
	
	packet_bytes_total += total_bytes;
		
	int op_fd=open("/var/tmp/aesdsocketdata",O_RDONLY);
	if(op_fd==-1)
	{
		perror("Could not open file for reading");
		syslog(LOG_ERR, "Could not open file for reading");
		exit(1);
	}
		
	char read_arr[packet_bytes_total];
	memset(read_arr,0,packet_bytes_total);
		
	int rd_status=read(op_fd,&read_arr,packet_bytes_total);
	if(rd_status==-1)
	{
		syslog(LOG_ERR, "Could not read bytes from the file");
		exit(1);
	}
	else if(rd_status < packet_bytes_total) 
	{
		syslog(LOG_ERR, "The bytes read do not match the bytes requested to be read");
		exit(1);
	}		
	send_status=send(accept_return,&read_arr,packet_bytes_total,0);
	if(send_status==-1)
	{
		syslog(LOG_ERR, "The data packets couldn't be send to the client");
		exit(1);
	}
		
	close(op_fd);
	free(store_data);
	syslog(LOG_INFO,"Closed connection with %s\n",inet_ntoa(client_addr.sin_addr));
	total_bytes=0;
	}

	closelog();
	return 0;
}
