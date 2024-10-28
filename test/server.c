#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <netinet/in.h>

#include<unistd.h>
#include<stdlib.h>

#define PORT 9002 //or 8080 or any other unused port value

int main()
{
	//create socket
	int server_socket;
	server_socket = socket(AF_INET , SOCK_STREAM,0);

	//check for fail error
	if (server_socket == -1) {
        printf("socket creation failed..\n");
        exit(EXIT_FAILURE);
    }

	//setsock
	int value  = 1;
	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value)); //&(int){1},sizeof(int)
	
	//define server address structure
	struct sockaddr_in server_address;
	bzero(&server_address,sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;


	//FOR client connections later in the code
	// Client socket address structures
	struct sockaddr_in client_address;

	// Stores byte size of server socket address
	socklen_t addr_size =  sizeof(client_address);
	/////////////////////////////

	//bind the socket to our specified IP and port
	if (bind(server_socket , 
		(struct sockaddr *) &server_address,
		sizeof(server_address)) < 0)
	{
		printf("socket bind failed..\n");
        exit(EXIT_FAILURE);
	}
	 
	//after it is bound, we can listen for connections
	if(listen(server_socket,5)<0){
		printf("Listen failed..\n");
		close(server_socket);
        exit(EXIT_FAILURE);
	}
	

	printf("Server is listening...\n");
	
	//to track number of connected clients
	int count = 0;

	while(1)
	{
		//// Accept clients and
		// store their information in client_address
		int client_socket = accept(
			server_socket, (struct sockaddr*)&client_address,
			&addr_size);

		// Displaying information of
		// connected client
		printf("Connection accepted from %s:%d\n",
			inet_ntoa(client_address.sin_addr),
			ntohs(client_address.sin_port));

		// Print number of clients
		// connected till now
		printf("Clients connected: %d\n\n",
			++count);

		
		int pid = fork(); //fork a child process

		if(pid == 0)   //if it is the child process
		 {
		 	close(server_socket); //close the copy of server/master socket in child process
		 	char buffer[256];
			while(1)
			{
				bzero(buffer,sizeof(buffer));
				int bytes = recv(client_socket,buffer,sizeof(buffer),0);
				if(bytes==0)   //client has closed the connection
				{
					printf("connection closed from client side \n");
					close(client_socket);
					exit(1); // terminate client program
				}
				printf("%s \n",buffer);
			}
		 }
		 else //if it is the parent process
		 {
		 	close(client_socket); //close the copy of client/secondary socket in parent process 
		 }
	}


	
	close(server_socket);

	return 0;
}
