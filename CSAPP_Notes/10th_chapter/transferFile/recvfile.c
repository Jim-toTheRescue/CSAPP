#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>


#define BUFSIZE 10485760 //10M
char fileBuf[BUFSIZE];
size_t fbsz = 0;

size_t buf2f(FILE* fp, char buf[], size_t sz);
int run(const char* port);//return file discriptor
size_t frecv(int fd, char* buf, size_t sz);
int Accept(int listenfd, struct sockaddr* sa, int* len);

int main(int argc, char *argv[])
{
	if (argc != 2){
	      fprintf(stderr, "Usage: %s port\n", argv[0]);
        	return -1;
	}
	int listenfd, connfd, len;
	listenfd = run(argv[1]);
	struct sockaddr_in client;
	char filename[256];
	size_t filesize, rsz = 0, filename_sz, wsz;
	ssize_t status;
	FILE* fp;
	
	if (listenfd < 0){
        	fprintf(stderr, "Error: %s\n", "Fail to run server...");
        	return -1;
    	}
	while (1){
		printf("Listening at 0.0.0.0: %s\n\n\n", argv[1]);
		connfd = Accept(listenfd, (struct sockaddr*)&client, &len);
		if (connfd == -1) continue;		
		frecv(connfd, (char*)&filename_sz, sizeof(size_t)); // read the size of filename from peer
		frecv(connfd, (char*)&filename, filename_sz); //read the filename from peer
		strcat(filename, ".backup");
		fp = fopen(filename, "wb");
		if (!fp){
			fprintf(stderr, "Fail to open %s\n", filename);
			close(connfd);
			continue;	
		}
		filesize = 0;
		// read file chunk length
		while((status = frecv(connfd, (char*)&fbsz, sizeof(fbsz))) != -1 && fbsz != 0){
			rsz = frecv(connfd, fileBuf, fbsz); //read file chunk
			//printf("recieve %d bytes\n", rsz); 
			wsz = buf2f(fp, fileBuf, rsz);
			filesize += rsz;
		}
		fclose(fp);
		close(connfd);
		printf("receive total %d bytes from peer\n", filesize);
	}
}

size_t frecv(int connfd, char* buf, size_t sz)
{
	int status;
	size_t rsz = 0;
	char* pbuf = buf;
	while(rsz < sz){
		status = read(connfd, buf+rsz, sz-rsz);
		if (status == 0){//connection broken by peer, EOF
           		printf("connection discarded by peer\n");
			exit(1);
     		}
		else if (status == -1){
           		fprintf(stderr, "broken connection\n");    
           		exit(1);
        	}
		rsz += status;
	}	
	return rsz;
}	



size_t buf2f(FILE *fp, char buf[], size_t sz)
{
	size_t n = 0, chunk = 1024;
	int ret = sz;
	char* pbuf = buf;
	while(sz > 0){
		if (sz < 1024) chunk = sz;
		n = fwrite(pbuf, 1, chunk, fp);
		if (n == chunk){
			pbuf += n;
			sz -= n;
		} 
		else{
			fprintf(stderr, "Fail to write...\n");
			exit(1);
		}
	}
	return ret - sz;
}


int run(const char* port)
{
	int status, listenfd, optval=1;
    	struct addrinfo hints, *res, *rp;
    	memset(&hints, '\0', sizeof(struct addrinfo));

    	hints.ai_family = AF_INET;
    	hints.ai_socktype = SOCK_STREAM;
    	hints.ai_protocol = 0;
    	hints.ai_flags = AI_PASSIVE;

    	status = getaddrinfo(NULL, port, &hints, &res);
    	if (status != 0){
        	fprintf(stderr, "Error: %s\n", "getaddrinf");
        	return -1;
     	}
    	for (rp = res; rp; rp = rp->ai_next){
        	listenfd =socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        	//Eliminates "Address already in use" error from bind
        	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));

        	if (listenfd < 0)
            	continue;
        
        	status = bind(listenfd, rp->ai_addr, rp->ai_addrlen);
        	if (status == 0)
            	break;
        
        	close(listenfd);
    	}
    	if(!rp){
        	fprintf(stderr, "Error: %s\n", "socket");
        	return -1;
    	}
    	freeaddrinfo(res);
    	status = listen(listenfd, 1024);
    	if (status == 0)
        	return listenfd;
    	else
        	return -1;
}


int Accept(int listenfd, struct sockaddr* client, int* len)
{
	char hostname[50], port[20];
	int connfd, status;
	
	connfd = accept(listenfd, (struct sockaddr*)&client, len);
      if (connfd == -1){
            fprintf(stderr, "Error: %s\n", "Fail to accept clients...");
            return -1;        
        }
	status = getnameinfo((struct sockaddr*)&client, sizeof(struct sockaddr_in), 
                                 hostname, sizeof(hostname), port, sizeof(port), 0);
     	if (status == 0){
    	     	printf("\nRequest from:\n %s:%s\n", hostname, port);
        }
	return connfd;
}






