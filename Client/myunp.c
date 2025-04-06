#include "myunp.h"

void Getaddrinfo(const char* restrict node, const char* restrict service, const struct addrinfo* restrict hints, struct addrinfo** restrict res)
{
	if (getaddrinfo(node, service, hints, res) != 0)
	{
		perror("ERROR - getaddrinfo()");
		exit(1);
	}
}

Sigfunc* Signal(int signo, Sigfunc* func)
{
	struct sigaction act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM)
	{
		act.sa_flags |= SA_RESTART;
	}

	if (sigaction(signo, &act, &oact) < 0)
	{
		return SIG_ERR;
	}
	return oact.sa_handler;
}

//helper function to Readline()
//RETURNS (1)->reading completed successfully, (0)->end of file reached, (-1)->reading error
static ssize_t my_read(int _fd, char* _dstbuff)
{
	static long int _read_cnt = 0;		//number of read data
	static char* _read_ptr;			//pointer to current position in _read_buf
	static char _read_buf[MAXLINE];		//read data buffer

	if (_read_cnt <= 0)			//if no data was read
	{
	_again:
		//READ
		_read_cnt = read(_fd, _read_buf, sizeof(_read_buf));

		//CHECK READING CORRECTNESS
		if (_read_cnt < 0)
		{
			//ERROR HANDLE
			if (errno == EINTR)	//interrputed by signal
			{
				goto _again;
			}
			else			//other errors
			{
				perror("ERROR - my_read()");
				return(-1);
			}
		}//END OF FILE HANDLE
		else if (_read_cnt == 0)
		{
			return(0);
		}
		_read_ptr = _read_buf;
	}

	_read_cnt--;
	*_dstbuff = *_read_ptr++;
	return(1);
}

ssize_t Readline(int _fd, void* _dstbff, size_t _maxlen)
{
	ssize_t _rc;			//result returned by my_read
	char _singleChar;		//single read char
	char* _ptr = _dstbff;		//pointer to _dstbuff

	int _n;				//loop counter
	for (_n = 1; _n < _maxlen; _n++)
	{
		//single char read
		_rc = my_read(_fd, &_singleChar);

		
		if (_rc == 1)		//reading successful
		{
			//copying the read character to the helper buffer and post-incrementing the pointer
			*_ptr++ = _singleChar;
			//if the read character is \0, the loop is interrupted and the line ends with a newline character
			if (_singleChar == '\n') break;
		}
		else if (_rc == 0)//end of file reached
		{
			if (_n == 1) return(0);	//if nothing was read return 0
			else break;		//if something was read break the loop
		}
		else 				//reading failed
		{
			perror("ERROR - Readline()");
			return(-1);
		}
	}

	*_ptr = '\0';				//add the end of the line at the end
	return(_n);				//return number of characters read (without \0))
}


size_t Readn(int _fd, void* _dstbuff, size_t _n)
{
	size_t _n_left = _n;	//number of bytes left to receive
	ssize_t _n_read = 0;	//number of bytes received

	while (_n_left > 0)
	{
		//READ
		_n_read = read(_fd, _dstbuff, _n_left);

		//ERROR HANDLING
		if (_n_read < 0)
		{
			if (errno == EINTR)		//if the error occurred due to a signal interruption, start again	
			{
				_n_read = 0;
			}
			else				//if another error occurs, terminate the program
			{
				perror("ERROR - read()");
				exit(1);
			}
		}
		else if (_n_read == 0)			//if this is the end of the file
		{
			break;
		}

		_n_left -= (size_t)_n_read;
		_dstbuff += _n_read;
	}
	return (_n - _n_left);
}


ssize_t Writen(int _fd, const void* _dstbuff, size_t _n)
{
	size_t _n_left = _n;		//number of bytes left to be sent
	ssize_t _n_written = 0;		//number of bytes transferred

	while (_n_left > 0)
	{
		//WRITE
		_n_written = write(_fd, _dstbuff, _n_left);		//sending and recording the number of bytes successfully transferred

		//ERROR HANDLING
		if (_n_written <= 0)
		{
			if (errno == EINTR) 				//if the error occurred due to a signal interruption, start again
			{
				_n_written = 0;
			}
			else						//if another error occurs, terminate the program
				perror("ERROR- write()");
				exit(1);
			}
		}

		_n_left -= (size_t) _n_written;		//updating the remaining bytes to be sent
		_dstbuff += _n_written;			//moving the pointer to the next buffer element
	}
	return _n_written;
}

pid_t Fork(void)
{
	pid_t _pid;

	if ((_pid = fork()) < 0)
	{
		perror("ERROR - fork()");
		exit(1);
	}

	return _pid;
}

void check_ip_arg(int _argc)
{
	if (_argc != 2)
	{
		perror("No server IP address entered");
		exit(1);
	}
}

int Socket(int _family, int _type, int _protocol)
{
	int _sockfd;
	if ((_sockfd = socket(_family, _type, _protocol)) < 0)
	{
		perror("ERROR - socket()");
		exit(1);
	}
	return _sockfd;
}

void Inet_pton(int _family, const char* restrict _src, void* restrict _dst)
{
	if (inet_pton(_family, _src, _dst) <= 0)
	{
		perror("ERROR - inet_pton()");
		exit(1);
	}
}

const char* Inet_ntop(int _family, const void* restrict _src, char* restrict _dst, socklen_t _size)
{
	if ((inet_ntop(_family, _src, _dst, _size)) == NULL)
	{
		printf("ERROR - inet_ntop()");
		exit(1);
	}
	return _dst;
}

void Bind(int _sockfd, const struct sockaddr* _addr, socklen_t _addrlen)
{
	if ((bind(_sockfd, _addr, _addrlen)) < 0)
	{
		perror("ERROR - bind()");
		exit(1);
	}
}

void Listen(int _sockfd, int _backlog)
{
	if ((listen(_sockfd, _backlog)) < 0)
	{
		perror("ERROR - listen()");
		exit(1);
	}
}

void Connect(int _sockfd, const struct sockaddr* _dstaddr, socklen_t _dstaddrlen)
{
	if ((connect(_sockfd, _dstaddr, _dstaddrlen)) < 0)
	{
		perror("ERROR - connect()");
		exit(1);
	}
}

int Accept(int _sockfd, struct sockaddr* restrict _addr, socklen_t* restrict _addrlen)
{
	int _sockfdCli;

	while (1)
	{
		if ((_sockfdCli = accept(_sockfd, _addr, _addrlen)) < 0)
		{
			if (errno == EINTR)
			{
				continue;	//retry accept() if interrupted by signal
			}
			else
			{
				perror("ERROR - accept()");
				exit(1);
			}
		}	
		break;				//break the loop if execution was successful
	}
	return _sockfdCli;
}

void Write(int _sockfd, const void* _buf, size_t _count)
{
	if ((write(_sockfd, _buf, _count)) < 0)
	{
		perror("ERROR - write()");
		exit(1);
	}
}

ssize_t Read(int _fd, void* _buf, size_t _count)
{
	ssize_t _recvsize;

	if ((_recvsize = read(_fd, _buf, _count)) < 0)
	{
		perror("ERROR - read()");
		exit(1);
	}
	return _recvsize;
}

void Close(int _sockfd)
{
	if ((close(_sockfd)) < 0)
	{
		perror("ERROR - close()");
		exit(1);
	}
}
