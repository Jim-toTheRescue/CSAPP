#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#define BUF_SIZE 4096
extern int getaddrinfo(const char* host, const char* service,
				const struct addrinfo* hints, struct addrinfo **results);
//return: socket descriptor
int request(const char* host, const char* port);

int main(int argc, char* argv[])
{
	int clientfd, i=0, status;
	char rbuf[BUF_SIZE], wbuf[BUF_SIZE], word[50], ch, gab[BUF_SIZE];
	ssize_t r, w;
	
	if (argc != 3){
		fprintf(stderr, "Usage: %s host port\n", argv[0]);
		return -1;
	}
	clientfd = request(argv[1], argv[2]);
	if (clientfd < 0) return -1;
	printf(">> ");
	while (fgets(wbuf, sizeof(wbuf), stdin) != NULL){
		i = 0;
		w = write(clientfd, wbuf, strlen(wbuf)+1);
		if (w == -1){
			fprintf(stderr, "Fail to send data...\n");
			return -1;
		}

		r = read(clientfd, rbuf, sizeof(rbuf));
		if (r == -1){
			return -1;
		} else if (r == 0){
			printf("connection discarded by peer\n");
			return 0;
		}
		printf(">> ");
		while(ch = rbuf[i++]) putchar(ch);
		//-------------------------------------------->>>>>>>>>>这两天最大的问题通过这一句解决，rbuf没有刷新，从socket  read的字符没有'\0'，猜测因为read以'\0'判断读取结束。 
		memset(rbuf, '\0', sizeof(rbuf)); //clear the rbuf for next read 
		printf("\n>> ");
	}
	close(clientfd);
	return 0;
}

int request(const char* host, const char* port)
{
	int sockfd, status;
	struct addrinfo hints, *res, *rp;
	const char* error;
	memset(&hints, '\0', sizeof(struct addrinfo));
	
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	status = getaddrinfo(host, port, &hints, &res);
	if (status != 0){
		error = gai_strerror(status);
		fprintf(stderr, "%s\n", error);
		return -1;
	}
	for (rp = res; rp; rp = rp->ai_next){
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd  < 0){
			continue;
		}
		status = connect(sockfd, rp->ai_addr, rp->ai_addrlen);
		if (status == 0){
			break;
		}
		close(sockfd);
	}
	freeaddrinfo(res);
	if (!rp){//Fail to connect...
		fprintf(stderr, "Fail to connect %s: %s\n", host, port);
		return -2;
	}
	return sockfd;
}
	
	
	
