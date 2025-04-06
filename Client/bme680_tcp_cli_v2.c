#define _POSIX_C_SOURCE 200809L
#include "myunp.h"
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

//#define MAXLINE 1023		//1023 + /0
#define PORT 9999		//SERVER PORT

int main(int argc, char** argv)
{
	check_ip_arg(argc);

	//DNS
	//entry in /etc/hosts
	//example:
	//<any IP>	esp32_serv.local
	
	struct addrinfo hints, *res;
    	char ipstr[INET_ADDRSTRLEN];

    	memset(&hints, 0, sizeof(hints));
    	hints.ai_family = AF_INET;
    	hints.ai_socktype = SOCK_STREAM;

   	Getaddrinfo(argv[1], NULL, &hints, &res);

    	struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    	inet_ntop(res->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));


	//CONNECTION SOCKET
	int connfd = Socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in connsock;
	socklen_t connsock_len = sizeof(connsock);
	memset(&connsock, 0, connsock_len);

	connsock.sin_family = AF_INET;
	connsock.sin_port = htons(PORT);
	connsock.sin_addr = ipv4->sin_addr;
	
	freeaddrinfo(res);

	//receive buffer
	char recvbuf[MAXLINE];	
	size_t recvbuf_size;

	//other variables
	const char errorMessage[17] = "Waiting in queue\0";
	char recvbuf_cut[MAXLINE];

	//CONNECTION
	Connect(connfd, (struct sockaddr*)&connsock, connsock_len);

	//READ FROM SERVER
	while ( (recvbuf_size = Readn(connfd, recvbuf, MAXLINE)) > 0)
	{
		recvbuf[recvbuf_size] = '\0';
		
		strncpy(recvbuf_cut, recvbuf, 16);
    		recvbuf_cut[17] = '\0';
		
		printf("SERVER: %s", recvbuf);

		if(strcmp(recvbuf_cut,errorMessage) == 0 )
		{
			printf("CLIENT: next try...\n\r");
			sleep(1);

		}
		else
		{
			printf("CLIENT: success!\n\r");
		}
	}
	
	exit(0);
}
