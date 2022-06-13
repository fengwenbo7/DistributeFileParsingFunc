#ifndef DISTRIBUTEFILEPARSE_SERVER_H
#define DISTRIBUTEFILEPARSE_SERVER_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <algorithm>
#include "specific_time.h"

using namespace std;

#define MAX_BUFFER_SIZE 1024

struct ClientInfo
{
    sockaddr_in client_addr;
    int client_fd;
    string client_ip;
};


class Server{
private:
    int listen_fd;
    int listen_port;
    int listen_size;
    sockaddr_in server_addr;
public:
    Server(int port,int size);
    void Init();
    void Listen();
    int GetListenFd();
    int AcceptConnect(ClientInfo& info);
    int Write(int sock_fd,char buf[]);
    int Read(int sock_fd,char buf[]);
    virtual ~Server()=default;
};

#endif