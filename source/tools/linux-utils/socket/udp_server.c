#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define SERVER_PORT 1234
#define BUF_SIZE	100

int main()
{
	int sockfd;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t addr_len;
	int num;
	char buf[BUF_SIZE];
	char msg[BUF_SIZE] = "Welcome to my server.";

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		perror("Creating socket failed.");
		exit(1);
	}

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
	server.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("Bind()error.");
		exit(1);
	}   

	addr_len = sizeof(client);
	while(1)  
	{
		num = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&client, &addr_len);                                   

		if (num < 0)
		{
			perror("recvfrom() error\n");
			exit(1);
		}

		buf[num] = '\0';
		printf("client msg : %s.\n", buf);
		printf("client info: %s:%d.\n",
				inet_ntoa(client.sin_addr), htons(client.sin_port)); 

		sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&client, addr_len);
		if(!strcmp(buf,"bye"))
		{
			break;
		}
	}
	close(sockfd);  

	return 0;
}
