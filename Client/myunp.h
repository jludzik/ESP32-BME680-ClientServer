#ifndef myunp
#define myunp

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/wait.h>

//----BASED ON unp.h W.R.Stevens -------

typedef void Sigfunc(int);

#ifndef MAXLINE
#define MAXLINE 1024
#endif


/**
 * wrapper function for getaddrinfo()
 * @param node
 * @param service
 * @param hints
 * @param res
*/
void Getaddrinfo(const char* restrict node, const char* restrict service, const struct addrinfo* restrict hints, struct addrinfo** restrict res);

/**
 * wrapper function for signal
 * @param signo
 * @param func
 * @return on success the previous signal handler, else the function will return SIG_ERR
*/
Sigfunc* Signal(int signo, Sigfunc* func);


/**
 * function reads lines char by char
 * @param _fd socket
 * @param _dstbuff transfer buffer
 * @param _maxlen max length of data
 * @return _n number of characters read,0 end of file no lines read,-1 read error
*/
ssize_t Readline(int _fd, void* _dstbff, size_t _maxlen);


/**
 * function reads _n bytes of data
 * @param _fd socket
 * @param _dstbuff receive buffer
 * @param _n size of received data
 * @return number of received bytes
*/
size_t Readn(int _fd, void* _dstbuff, size_t _n);


/**
 * function sends _n bytes of data
 * @param _fd socket
 * @param _dstbuff transfer buffer
 * @param _n size of data to send
 * @return number of data sent
*/
ssize_t Writen(int _fd, const void* _dstbuff, size_t _n);


//funkcja tworzy proces potomny, zwraca pid procesu potomnego w przodku, zwraca 0 w potomku
/**
 * wrapper function for fork(),
 * function creates a child process
 * @return pid of the child process in the parent, returns 0 in the child
*/
pid_t Fork(void);


/**
 * function checking for an argument
 * @param _argc
*/
void check_ip_arg(int _argc);


/**
 * wrapper function for socket(),
 * if the socket creation is successful, it returns the descriptor,
 * otherwise it terminates the current task
 * @param _family
 * @param _type
 * @param _protocol
 * @return created socket
*/
int Socket(int _family, int _type, int _protocol);


/**
 * wrapper function for inet_pton(),
 * function converts IP address from text to binary number
 * @param _family
 * @param _src
 * @param _dst
*/
void Inet_pton(int _family, const char* restrict _src, void* restrict _dst);


/**
 * wrapper function for inet_ntop(),
 * function converts IP address from binary number to text
 * @param _family
 * @param _src
 * @param _dst
 * @param _size
 * @return non-null pointer to _dst.
*/
const char* Inet_ntop(int _family, const void* restrict _src, char* restrict _dst, socklen_t _size);


/**
 * wrapper function for bind(),
 * function assigns an address structure to a socket
 * @param _sockfd
 * @param _addr
 * @param _addrlen
*/
void Bind(int _sockfd, const struct sockaddr* _addr, socklen_t _addrlen);


/**
 * wrapper function for listen(),
 * function starts listening on the socket
 * @param _sockfd
 * @param _backlog
*/
void Listen(int _sockfd, int _backlog);


/**
 * wrapper function for connect(),
 * function initiates a connection on the socket
 * @param _dstaddr
 * @param _dstaddrlen
*/
void Connect(int _sockfd, const struct sockaddr* _dstaddr, socklen_t _dstaddrlen);


/**
 * wrapper function for accept(),
 * function accepts a connection on the listening socket
 * @param _sockfd
 * @param _addr
 * @param _addrlen
 * @return socket of connected host
*/
int Accept(int _sockfd, struct sockaddr* restrict _addr, socklen_t* restrict _addrlen);


/**
 * wrapper function for write(),
 * function sends the buffer
 * @param _sockfd
 * @param _buf
 * @param _count
 * @return number of received bytes
*/
void Write(int _sockfd, const void* _buf, size_t _count);


/**
 * wrapper function for read(),
 * function reads data
 * @param _fd
 * @param _buf
 * @param _count
 * @return number of received bytes
*/
ssize_t Read(int _fd, void* _buf, size_t _count);


/**
 * wrapper function for clsoe(),
 * function close a file descriptor
 * @param _sockfd
*/
void Close(int _sockfd);


#endif
