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

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;
};

#endif