#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#define BUF_SIZE 4096

extern int getaddrinfo(const char* host, const char* service,
				const struct addrinfo* hints, struct addrinfo **results);

int run(const char* port);
void *echo(void* pfd);
ssize_t safe_read(int fd, char *usrbuf, size_t sz);
ssize_t safe_write(int fd, char *usrbuf, size_t sz);

int main(int argc, char *argv[])
{
    int status, connfd, listenfd, len;
    uint32_t hexip;
    struct sockaddr_in client;
    char buff[50], hostname[50], port[50];
    pthread_t tid;

    if (argc != 2){
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        return -1;
    }
    listenfd = run(argv[1]);
    if (listenfd < 0){
        fprintf(stderr, "Error: %s\n", "Fail to run server...");
        return -1;
    }
    while (1){
        //因为阻塞在这里，程序本质上不是并发的，目前只是在完成了连接之后由独立线程对连接进行服务
        //应该预线程才可达到并发效果，也就是把线程创建了等待连接。
        connfd = accept(listenfd,  (struct sockaddr*)&client, &len);
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
        status = pthread_create(&tid, NULL, echo, &connfd);
        if (status==0){
            printf("Thread %d: created\n\n", tid);
        }
        pthread_detach(tid);
    }
    close(listenfd);

    return 0;
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

void *echo(void *pfd)
{
    char buff[BUF_SIZE], reply[BUF_SIZE], ch, gab[BUF_SIZE];
    int connfd = *(int*)pfd;
    int i = 0, status;

    while (1){
        i = 0;
        status = read(connfd, buff, sizeof(buff));
        if (status == 0){//connection broken by peer, EOF
            printf("connection discarded by peer\n");
            close(connfd);
            break;
        } else if (status == -1){
            fprintf(stderr, "broken connection\n");    
            exit(1);
        }
        printf("\n>> ");
        while(ch = buff[i]) {
            putchar(ch);
            if (isalpha(ch))
                buff[i] = toupper(buff[i]);
            ++i;   
        }
        
        status = write(connfd, buff, status);
        memset(buff, '\0', sizeof(buff)); //clear the buff for next read
        if (status == -1) {
            printf("connection discarded by peer\n");
            exit(1);
        }
    }
    return NULL;
}


/*bugs
>> client发送a, an, the, a，echo的结果一次为a, an, the, ahe  ---->问题时因为socket的内核缓冲区的数据没有刷新，残留有上一次的数据，
可能解决方案：在echo函数中，server（echo函数113行中）和client的read函数，每次都反复取至发生EOF再停止，因此不再能通过read函数判断对方是否已经断开，
需要更改read 返回0 的处理逻辑，不当作已经断开，而是在write中判断对方是否已经断开。
*/ 




