#ifndef DISTRIBUTEDFILEPARSING_DISTRIBUTION_SERVER_H
#define DISTRIBUTEDFILEPARSING_DISTRIBUTION_SERVER_H

#include "../../src_units/server.h"
#include <vector>
#include <thread>

enum ClientState{
    Waiting,
    Transmiting,
};

typedef struct{
    ClientInfo client_info;
    ClientState client_state;
}ClientNode;

class DistributionServer{
private:
    Server server;
    vector<ClientNode> client_node;//to save all the workstations that connect to task manager server
    bool isthreaded;
public:
    DistributionServer(int port,int size);
    int GetListenFd();
    ClientNode GetClientNode(int fd) const;

};

#endif