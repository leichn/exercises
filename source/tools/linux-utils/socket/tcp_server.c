#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define  PORT 1234
#define  BACKLOG 1

int main()
{
	int listenfd, connectfd;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t addrlen;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Creating  socket failed.");
		exit(1);
	}

	int opt =SO_REUSEADDR;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&server,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(PORT);
	server.sin_addr.s_addr= htonl (INADDR_ANY);

	if (bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("bind error.");
		exit(1);
	}   

	if (listen(listenfd, BACKLOG) == -1)
	{  /* calls listen() */
		perror("listen() error\n");
		exit(1);
	}

	addrlen =sizeof(client);
	if ((connectfd = accept(listenfd, (struct sockaddr*)&client, &addrlen)) == -1)
	{
		perror("accept()error\n");
		exit(1);
	}
	printf("Yougot a connection from cient's ip is %s, prot is %d\n", inet_ntoa(client.sin_addr), htons(client.sin_port));
	send(connectfd, "Welcometo my server.\n", 22, 0);
	close(connectfd);
	close(listenfd);
	return 0;
}
