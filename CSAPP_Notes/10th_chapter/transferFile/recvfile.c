#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>


#define BUFSIZE 10485760 //10M
char fileBuf[BUFSIZE];
size_t fbsz = -1;

size_t buf2f(const char filename[], char buf[], size_t sz);
int run(const char* port);//return file discriptor
size_t recvpic(int fd);
int recvsz(int connfd);

int main(int argc, char *argv[])
{
	if (argc != 2){
	      fprintf(stderr, "Usage: %s port\n", argv[0]);
        	return -1;
	}
	int listenfd, connfd, len, status;
	listenfd = run(argv[1]);
	struct sockaddr_in client;
	char hostname[50], port[20];
	size_t filesize, rsz = 0;
	
	if (listenfd < 0){
        	fprintf(stderr, "Error: %s\n", "Fail to run server...");
        	return -1;
    	}
	while (1){
		filesize = 0;
		connfd = accept(listenfd, (struct sockaddr*)&client, &len);
      	if (connfd == -1){
      	      fprintf(stderr, "Error: %s\n", "Fail to accept clients...");
      	      return -1;        
        	}
        	printf("LIstening at 0.0.0.0: %s\n\n\n", argv[1]);
        	status = getnameinfo((struct sockaddr*)&client, sizeof(struct sockaddr_in), 
                                    hostname, sizeof(hostname), port, sizeof(port), 0);
        	if (status == 0){
            	printf("\nRequest from:\n %s:%s\n", hostname, port);
        	}

		while((status = recvsz(connfd)) != -1 && fbsz != -1){
	/*
		if (status == -1) {
			close(connfd);
			continue;
		}
	*/
			rsz = recvpic(connfd);
			filesize += rsz;
		}
		printf("receive total %d bytes from peer\n", filesize);
	}
}

//recieve the size of the following message
int recvsz(int connfd)
{
	int status;
	status = read(connfd, &fbsz, sizeof(size_t));
     	if (status == 0){//connection broken by peer, EOF
           	printf("connection discarded by peer: no size info recieved\n");
           	close(connfd);
		return -1;
     	}
	else if (status == -1){
           	fprintf(stderr, "broken connection:no size info recieved\n");    
           	return -1;
        }	
}

size_t recvpic(int connfd)
{
	int status = 0;
	size_t wsz, rsz=0;
	char* pbuf = fileBuf;
	while(rsz < fbsz){
		status = read(connfd, pbuf+rsz, fbsz-rsz);
		rsz += status;
     		if (status == 0){//connection broken by peer, EOF
            	printf("connection discarded by peer\n");
            	close(connfd);
			break;
     		}
		else if (status == -1){
            	fprintf(stderr, "broken connection\n");    
            	exit(1);
        	}
		//printf("status = %d\n", status);
	}
	//printf("receive %d bytes from peer\n", rsz);
	wsz = buf2f("test.pdf", fileBuf, rsz);
	if (wsz == -1){
		fprintf(stderr, "Fail to save...\n");
		return -1;
	} else {
		printf("Save %d bytes to test.pdf\n", wsz);
	}
	return wsz;	
}




size_t buf2f(const char filename[], char buf[], size_t sz)
{
	FILE* fp = fopen(filename, "ab");
	if (!fp){
		fprintf(stderr, "Fail to open %s\n", filename);
		return -1;	
	}
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
			fprintf(stderr, "Fail to write %s\n", filename);
			fclose(fp);
			return -1;
		}
	}
	fclose(fp);
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

        if (listenfd < 0){
            continue;
        }
        status = bind(listenfd, rp->ai_addr, rp->ai_addrlen);
        if (status == 0){
            break;
        }
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
