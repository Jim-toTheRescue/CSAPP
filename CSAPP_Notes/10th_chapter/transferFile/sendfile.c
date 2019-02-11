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

#define MAX_FILENAME_LEN 128
#define BUFSIZE 10485760 //10M
char fileBuf[BUFSIZE];
size_t fbsz = 0;	
size_t f2buf(FILE* fp, char buf[], size_t bufsize);
int request(const char* host, const char* port);
ssize_t fsend(int sockfd, char* buf, size_t sz);

int main(int argc, char* argv[])
{
	if (argc != 3){
		fprintf(stderr, "Usage: %s host port\n", argv[0]);
		return -1;
	}
	int sockfd;
	char filename[MAX_FILENAME_LEN];
	ssize_t wsz, total = 0;
	size_t len;
	FILE* fp = NULL;

	sockfd = request(argv[1], argv[2]);
	if (sockfd < 0){
		fprintf(stderr, "Fail to connect %s:%s\n", argv[1], argv[2]);
		return -1;
	}
	
	while(fgets(filename, sizeof(filename)-1, stdin) != NULL){
		len = strlen(filename);
		if (filename[len-1] == '\n') filename[len-1] = '\0';
		fp = fopen(filename, "rb");
		if (!fp){
			fprintf(stderr, "Fail to open %s\n", filename);
			continue;
		}
		len += 1;//count the '\0'
		fsend(sockfd, (char*)&len, sizeof(len));//send filename length
		fsend(sockfd, filename, len); // send filename and '\0'
		while((fbsz = f2buf(fp, fileBuf, BUFSIZE)) > 0){
			printf("%u\n", fbsz);
			fsend(sockfd, (char*)&fbsz, sizeof(fbsz));//send file chunk length
			wsz = fsend(sockfd, fileBuf, fbsz); // send file chunk
			printf("%d bytes sent\n", wsz);
			total += wsz;
		}
		fbsz = 0; // send end flag
		printf("Done, send total %ld bytes to peer\n", total);
		fsend(sockfd,(char*)&fbsz, sizeof(fbsz));
		fclose(fp);
	}
	return 0;
}

/*
 * fd : file descriptor
 * buf: memory buffer with infomation to be sent to the socket
 * sz: the size to send, should be less than BUFSIZE;
 */
ssize_t fsend(int sockfd, char* buf, size_t sz)
{
	ssize_t wsz;//write size

	wsz = write(sockfd, buf, sz);
	if (wsz == -1){
		fprintf(stderr, "Fail to send data\n");
		exit(1);
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

/*
使用全局变量导致函数的接口很不统一，也使得函数的的用途过于单一，
比如sendpic只能用于发送fileBuf缓冲区，应该修改接口使其更加通用。

可以把发送和接受都抽象成单一函数，发送格式为：
message length + message body
*/
