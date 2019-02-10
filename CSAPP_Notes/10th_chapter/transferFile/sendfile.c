/*
Shortage:
>> This program reads the file to memory(fileBuf) first, and write it to the sockfd;
Usage:
>>./sendpic addr port
>> read pic // read a picture "pic" to buf from current directory
>> send //send the pic in the buf to peer
>> send pic //read and send pic to peer 
*/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //getaddrinfo, socket
#include <netdb.h> //getaddrinfo
#include <sys/types.h> //getaddrinfo ...
#include <stdlib.h>

#define MAX_CMD_LEN 128
#define BUFSIZE 10485760 //10M
char fileBuf[BUFSIZE];
size_t fbsz = -1;	
size_t f2buf(FILE* fp, char buf[], size_t bufsize);
ssize_t sendpic(int sockfd);
int request(const char* host, const char* port);
ssize_t sendsz(int sockfd);

int main(int argc, char* argv[])
{
	if (argc != 3){
		fprintf(stderr, "Usage: %s host port\n", argv[0]);
		return -1;
	}
	int sockfd;
	char cmd[MAX_CMD_LEN];
	ssize_t wsz;
	size_t len;
	FILE* fp = NULL;
	
	sockfd = request(argv[1], argv[2]);
	if (sockfd < 0){
		fprintf(stderr, "Fail to connect %s:%s\n", argv[1], argv[2]);
		return -1;
	}
	while(fgets(cmd, sizeof(cmd)-1, stdin) != NULL){
		len = strlen(cmd);		
		if (cmd[len-1] == '\n') cmd[len-1] = '\0';

		if (!strcmp(cmd, "send")){ // cmd == "send", send the fileBuf to peer
			if (!fp){
				fprintf(stderr, "key in the filename first\n");
				continue;
			}
			while((fbsz = f2buf(fp, fileBuf, BUFSIZE)) > 0){
				sendsz(sockfd);
				wsz = sendpic(sockfd);
			}
			fbsz = -1;//Finish sending, send fbsz = -1 to inform peer.
			sendsz(sockfd);
			fclose(fp);
		}
		else {//cmd != "send", consider it a filename
			fp = fopen(cmd, "rb");
			if (!fp){
				fprintf(stderr, "Fail to open %s\n", cmd);		
			}
		}	
	}
	return 0;
}

//to tell peer the size of the following message
ssize_t sendsz(sockfd)
{
	ssize_t sz;

	sz = write(sockfd, &fbsz, sizeof(fbsz));
	if (sz < 0){
		fprintf(stderr, "Fail to send size...\n");
		exit(1);
	}
	return sz;
}

ssize_t sendpic(int sockfd)
{
	ssize_t wsz;

	wsz = write(sockfd, fileBuf, fbsz);
	fbsz = -1;
	if (wsz == -1){
		fprintf(stderr, "Fail to send data...\n");
		exit(1);			
	} else {
		printf("Send %d bytes to peer\n", wsz);
	}
	return wsz;			
}

size_t f2buf(FILE* fp, char buf[], size_t bufsize)
{
	size_t n = 0, sz = 0;
	char* pbuf = buf;
	while((n = fread(pbuf, 1, 1024, fp)) > 0){
		pbuf += n;
		sz += n;
		if (sz > bufsize - 1024)
			break;
	}
	if (ferror(fp)){
		sz = -1;
	}
	return sz;	 
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
