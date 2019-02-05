#ifndef __RIO
#define __RIO
//对Unix_IO系统调用的封装，通过使用用户态缓存减少系统调用次数
#include <unistd.h>//read, write
#include <errno.h> //errno, EINTR
#include <string.h> //memcpy

#define RIO_BUFSIZE 8192
//无缓冲的鲁棒版本。
//返回：若成功返回传送的字节数，所EOF返回0（只对rio_readn而言），若出错则为-1
ssize_t rio_readn(int fd, void* usrbuf, size_t n);
ssize_t rio_writen(int fd, void* usrbuf, size_t n);

//带缓冲的鲁棒版本
struct rio_t {
	int rio_fd; //文件描述符
	int rio_cnt;//rio_buf中未读取的字节数
	char* rio_bufptr; //指向rio_buf中下一个被读取的字节
	char rio_buf[RIO_BUFSIZE];
}
void rio_readinitb(rio_t *rp, int fd);
//返回：若成功返回传送的字节数，所EOF返回0（只对rio_readn而言），若出错则为-1
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);
//rio_read时系统函数read的带缓冲版本
ssize_t rio_read(rio_t *rp, void *usrbuf, size_t n);

/*以下函数定义*/
/*没缓冲版本输入输出函数定义*/
ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
	ssize_t nread;
	size_t nleft = n;
	char *bufp = usrbuf;

	while (nleft > 0){
		if ((nread = read(fd, bufp, n)) < 0) {
			if (errno == EINTR)//Interrupted by sig handler return and call read again
				nread = 0;
			else 
				return -1;
		} 
		else if (nread == 0){
			break;
		}
		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft)
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
	ssize_t nwritten;
	size_t nleft = n;
	char *bufp = usrbuf;

	while (nleft > 0){
		if ((nwritten = write(fd, bufp, n) < 0){
			if (errno == EINTR)
				nwritten = 0;
			else 
				return -1;
		}
		nleft -= nwritten;
		bufp += nwritten;
	}
	return n;
}
/*以下带缓冲版本的输入函数定义*/
void rio_readinitb(rio_t *rp, int fd)
{
	rp->rio_fd = fd;
	rp->rio_cnt = 0;
	rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_read(rio_t *rp, void *usrbuf, size_t n)
{
	int cnt;
	while (rp->rio_cnt <= 0){//如果rp中的缓冲区空了
		rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, 
					sizeof(rp->rio_buf));
		if (rp->rio_cnt < 0){
			if (errno != EINTR)
				return -1;
		}
		else if (rp->rio_cnt == 0)//EOF*
			return 0;
		else
			rp->rio_bufptr = rp->rio_buf;
	}
	cnt = n;
	if (rp->rio_cnt < n)
		cnt = rp->rio_cnt;
	memcpy(usrbuf, rp->rio_buf, cnt);
	rp->rio_bufptr += cnt;
	rp->rio_cnt -= cnt;
	return cnt;		
}	

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
	int n, rc;
	char c, *bufp = usrbuf;
	
	for (n = 1; n < maxlen; ++n){
		if ((rc = rio_read(rp, &c, 1) == 1){
			*bufp++ = c;
			if (c == '\n'){
				n++;
				break;
			}
		}
		else if (rc == 0){
			if (n == 1)
				return 0;//EOF, 未读到数据
			else
				break; //EOF，读到了一部分数据
		}
		else 
			return -1; //read 出错
	}
	*bufp = 0;
	return n-1;
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
	size_t nleft = n;
	ssize_t nread;
	char *bufp = usrbuf;
	
	while (nleft > 0){
		if ((nread = rio_read(rp, bufp, nleft) < 0)
			return -1;
		else if (nread == 0)
			break; //EOF
		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft);
}

#endif













