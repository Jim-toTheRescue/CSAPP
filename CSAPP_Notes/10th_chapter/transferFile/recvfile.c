#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#define BUFSIZE 10485760 //10M
#define MAX_FILENAME_LEN 256
char fileBuf[BUFSIZE];
void sigchld_handler(int signum);//SIGCHLD handler to collect the zombie child processes
size_t buf2f(FILE* fp, char buf[], size_t sz);
int run(const char* port);//return file discriptor
size_t frecv(int fd, char* buf, size_t sz);
int Accept(int listenfd, struct sockaddr* sa, int* len);
int fsaveAs(int connfd);
void printProc(double cur, double total);

int main(int argc, char *argv[])
{
	if (argc != 2){
	      fprintf(stderr, "Usage: %s port\n", argv[0]);
        	return -1;
	}
	int listenfd, connfd, len, status;
	struct sockaddr_in client;
	char filename[256];
	pid_t cpid;
	
	if (signal(SIGCHLD, sigchld_handler) == SIG_ERR){
		fprintf(stderr, "Fail to setup SIGCHLD handler");
		exit(1);
	}
	listenfd = run(argv[1]);
	if (listenfd < 0){
        fprintf(stderr, "Error: %s\n", "Fail to run server...");
        exit(1);
    }
	while(1){
		printf("Listening at 0.0.0.0: %s\n\n", argv[1]);
		connfd = Accept(listenfd, (struct sockaddr*)&client, &len);
		if (connfd == -1) continue;	
		if ((cpid = fork()) == 0){
			fsaveAs(connfd);
			close(connfd);
			exit(1);
		}
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
			return -1;
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
		if (sz < chunk) chunk = sz;
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

//save recieving file as the filename recieved from peer
int fsaveAs(int connfd)
{
	FILE* fp;
	size_t fnameSize, fileSize, wsz, rsz, totalr = 0, fbsz = BUFSIZE;//fbsz is the expected reading size for one round
	int status;
	char filename[MAX_FILENAME_LEN];

	status = frecv(connfd, (char*)&fnameSize, sizeof(size_t)); //read filename size from peer
	if (status == -1){
		fprintf(stderr, "Fail to read filename size\n");
		return -1;
	}
	status = frecv(connfd, (char*)&filename, fnameSize); //read the filename from peer
	if (status == -1){
		fprintf(stderr, "Fail to read filename\n");
		return -1;
	}
	fp = fopen(filename, "wb"); //open/create the file to save recieved data in binary mode
	if (!fp){
		fprintf(stderr, "Fail to open %s\n", filename);
		return -1;
	}
	//recieve the size info about the file about to be recieved;
	status= frecv(connfd, (char*)&fileSize, sizeof(fileSize));
	if (status == -1){
		fprintf(stderr, "Fail to get the size info\n");
		return -1;
	}
	while (totalr < fileSize){
		if (fileSize - totalr < fbsz) fbsz = fileSize - totalr;
		rsz = frecv(connfd, fileBuf, fbsz);//fileBuf: a global buffer for connfd
		wsz = buf2f(fp, fileBuf, rsz); //write the just recieving rsz bytes to the file fp;
		totalr += rsz;
		printProc(totalr, fileSize);
	}
	printf("Recieve total %d bytes from peer\n", totalr);
	fclose(fp);
	return 0;
}

void sigchld_handler(int signum)
{
	if (wait(NULL) < 0){
		fprintf(stderr, "Fail to collect zombie child processes\n");
	}
	//printf("Collecting zombie processes...\n");
}

//打印进度百分比
void printProc(double cur, double total)
{
	printf("\33[2K\r");
	printf("进度: %.2lf%%", cur/total * 100);
	fflush(stdout);
}



