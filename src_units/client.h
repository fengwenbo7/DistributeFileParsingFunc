#ifndef DISTRIBUTEDFILEPARSING_CLIENT_H
#define DISTRIBUTEDFILEPARSING_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "specific_time.h"
using namespace std;

#define MAX_BUFFER_SIZE 1024

class Client{
private:
    int sock_fd;
    struct sockaddr_in client_addr;
    char server_ip[16];
    int server_port;
public:
    Client(char* server_ip,int server_port);
    int Connect();
    char* GetServerIP();
    int GetServerPort();
    int Write(char* buf);
    int Read(char* buf);
    int Close();
    virtual ~Client()=default;
};

#endif