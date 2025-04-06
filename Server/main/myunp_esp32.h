#ifndef myunp_esp32
#define myunp_esp32

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "esp_log.h"
#include "esp_event.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netif.h"

#ifndef MAXLINE
#define MAXLINE 1024
#endif // !MAXLINE

//based on unp.h R.Stevens

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
 * wrapper function for accept(),
 * function accepts a connection on the listening socket
 * @param _sockfd
 * @param _addr
 * @param _addrlen
 * @return socket of connected host
*/
int Accept(int _sockfd, struct sockaddr* restrict _addr, socklen_t* restrict _addrlen);

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

/**
 * function reads lines char by char
 * @param _fd socket
 * @param _dstbuff transfer buffer
 * @param _maxlen max length of data
 * @return _n number of characters read,0 end of file no lines read,-1 read error
*/
ssize_t Readline(int _fd, void* _dstbff, size_t _maxlen);

#endif