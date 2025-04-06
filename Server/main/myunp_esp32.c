#include "myunp_esp32.h"

int Socket(int _family, int _type, int _protocol)
{
	int _sockfd;
	if ((_sockfd = socket(_family, _type, _protocol)) < 0)
	{
		ESP_LOGI("TCP_SERVER_ERROR","ERROR - socket()");
        vTaskDelete(NULL);
	}
	return _sockfd;
}

void Bind(int _sockfd, const struct sockaddr* _addr, socklen_t _addrlen)
{
	if ((bind(_sockfd, _addr, _addrlen)) < 0)
	{
		ESP_LOGI("TCP_SERVER_ERROR","ERROR - bind()");
        close(_sockfd);
        vTaskDelete(NULL);
	}
}

void Listen(int _sockfd, int _backlog)
{
	if ((listen(_sockfd, _backlog)) < 0)
	{
        ESP_LOGI("TCP_SERVER_ERROR","ERROR - listen()");
        vTaskDelete(NULL);
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
				ESP_LOGI("TCP_SERVER_ERROR","ERROR - accept()");
                close(_sockfd);
                vTaskDelete(NULL);
			}
		}	
		break;	//break the loop if execution was successful
	}
	return _sockfdCli;
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
			if (errno == EINTR)	//if the error occurred due to a signal interruption, start again
			{
				_n_read = 0;
			}
			else				//if another error occurs, terminate the program
			{
				ESP_LOGI("TCP_SERVER_ERROR","ERROR - read()");
                vTaskDelete(NULL);
            }
		}
		else if (_n_read == 0)	//if this is the end of the file
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
			if (errno == EINTR) //if the error occurred due to a signal interruption, start again
			{
				_n_written = 0;
			}
			else				//if another error occurs, terminate the program
			{
				ESP_LOGI("TCP_SERVER_ERROR","ERROR - write()");
				close(_fd);
                vTaskDelete(NULL);
			}
		}

		_n_left -= (size_t) _n_written;		//updating the remaining bytes to be sent
		_dstbuff += _n_written;				//moving the pointer to the next buffer element
	}
	return _n_written;
}

static ssize_t my_read(int _fd, char* _dstbuff)
{
	static long int _read_cnt = 0;		//number of read data
	static char* _read_ptr;				//pointer to current position in _read_buf
	static char _read_buf[MAXLINE];		//read data buffer

	if (_read_cnt <= 0)	//if no data was read
	{
	_again:
		//READ
		_read_cnt = read(_fd, _read_buf, sizeof(_read_buf));

		//CHECK READING CORRECTNESS
		if (_read_cnt < 0)
		{
			//ERROR HANDLE
			if (errno == EINTR)		//interrputed by signal
			{
				goto _again;
			}
			else					//other errors
			{
				ESP_LOGI("TCP_SERVER_ERROR","ERROR - my_read()");
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
	char* _ptr = _dstbff;	//pointer to _dstbuff

	int _n;					//loop counter
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
		else if (_rc == 0) //end of file reached
		{
			if (_n == 1) return(0);	//if nothing was read return 0
			else break;				//if something was read break the loop
		}
		else 				//reading failed
		{
			ESP_LOGI("TCP_SERVER_ERROR","ERROR - Readline()");
			close(_fd);
			vTaskDelete(NULL);
		}
	}

	*_ptr = '\0';		//add the end of the line at the end
	return(_n);			//return number of characters read (without \0))
}