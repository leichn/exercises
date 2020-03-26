#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SERVER_PORT 1234
#define BUF_SIZE	100

int main(int argc, char *argv[])
{
	int sockfd, num;
	char buf[BUF_SIZE];
	struct hostent *he;
	struct sockaddr_in server, peer;
	socklen_t addr_len = sizeof(server);

	if (argc !=3)
	{
		printf("Usage: %s <ip> <message>\n",argv[0]);
		exit(1);
	}

	if ((he = gethostbyname(argv[1]))==NULL)
	{
		printf("gethostbyname() error\n");
		exit(1);
	}

	if ((sockfd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
	{
		printf("socket() error\n");
		exit(1);
	}

	const int opt = 1;
	int nb = 0;
	nb = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
	if (nb == -1)
	{
		printf("set socket error...\n");
		exit(1);
	}

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
	server.sin_addr = *((struct in_addr *)he->h_addr);
	sendto(sockfd, argv[2], strlen(argv[2]), 0, (struct sockaddr *)&server, addr_len);

	while (1)
	{
		num = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&peer, &addr_len);
		if (num < 0)
		{
			printf("recvfrom() error\n");
			exit(1);
		}

		buf[num]='\0';
		printf("server msg : %s\n", buf);
		printf("server info: %s:%d\n",
				inet_ntoa(peer.sin_addr), htons(peer.sin_port)); 

		break;
	}

	close(sockfd);

	return 0;
}
