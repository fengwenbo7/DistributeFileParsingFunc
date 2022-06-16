/*
fenwgenbo 2022-06
*/

#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>      
#include <atomic>
#include "http_request.h"
#include "http_response.h"

class HttpConn{
public:
    HttpConn();
    ~HttpConn();
    void init(int sockFd, const sockaddr_in& addr);
    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);
    void Close();
    int GetFd() const;
    int GetPort() const;
    const char* GetIP() const;
    sockaddr_in GetAddr() const;
    bool process();

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;

private:
    int fd_;
    struct  sockaddr_in addr_;
    bool isClose_;
    int iovCnt_;
    struct iovec iov_[2];
};

#endif