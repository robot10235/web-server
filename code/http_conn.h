#ifndef HTTPCONN_H
#define HTTPCONN_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/uio.h>
#include "locker.h"

class http_conn {
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    /* only support GET now */
    enum METHOD {GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH};
    /* server fsm has 3 states */
    enum CHECK_STATE {CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT};
    enum HTTP_CODE {NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE,
                    FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION};
    /* client finite state machine has 3 states */
    enum LINE_STATUS {LINE_OK = 0, LINE_BAD, LINE_OPEN};

public:
    http_conn() { }
    ~http_conn() { }

public:
    /* init connection */ 
    void init(int sockfd, const sockaddr_in& addr);
    void close_conn(bool real_close = true);
    void process();     /* subthread process */
    bool read();
    bool write();

private:
    void init();
    /* analyze http request */
    HTTP_CODE process_read();
    /* http respond */
    bool process_write(HTTP_CODE ret);

    /* used for process_read*/
    HTTP_CODE parse_request_line(char* text);
    HTTP_CODE parse_headers(char* text);
    HTTP_CODE parse_content(char* text);
    HTTP_CODE do_request();
    char* get_line() { return m_read_buf + m_start_line; }
    LINE_STATUS parse_line();

    /* used for process_write */
    void unmap();
    bool add_response(const char* format, ...);
    bool add_content(const char* content);
    bool add_status_line(int status, const char* title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    /* all events happened in the socket are registered in the same epoll kernel list */
    static int m_epollfd;
    static int m_user_count;

private:
    /* socket fd */
    int m_sockfd;
    /* client addr */
    sockaddr_in m_address;

    /* read buffer */
    char m_read_buf[READ_BUFFER_SIZE];
    /* idx of the next position of read client data */
    int m_read_idx;
    /* the start idx of checking */
    int m_checked_idx;
    /* the start line of the read buffer */
    int m_start_line;
    
    /* write buffer */
    char m_write_buf[WRITE_BUFFER_SIZE];
    /* send how many bytes of write buffer */
    int m_write_idx;

    /* the current state of the server */
    CHECK_STATE m_check_state;
    /* http method */
    METHOD m_method;

    /* file path */
    char m_real_file[FILENAME_LEN];
    /* target file name */
    char* m_url;
    /* http version 1.1*/
    char* m_version;
    /* host */
    char* m_host;
    /* http request msg len */
    int m_content_length;
    /* http request needs long connection */
    bool m_linger;

    /* started location of the requested file in mmap */
    char* m_file_address;
    /* target file state */
    struct stat m_file_stat;
    /* writev */
    struct iovec m_iv[2];
    int m_iv_count;
};

#endif