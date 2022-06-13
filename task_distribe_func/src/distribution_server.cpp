#include "distribution_server.h"

DistributionServer::DistributionServer(int port,int size):server(port,size),client_node(),isthreaded(false){
    this->server.Listen();
}

int DistributionServer::GetListenFd(){
    return this->server.GetListenFd();
}

ClientNode DistributionServer::GetClientNode(int fd) const{
    auto client_it=find_if(client_node.begin(),client_node.end(),[fd](const ClientNode& cli){return cli.client_info.client_fd==fd;});
    return *client_it;
}

int DistributionServer::AcceptConnection(ClientNode& client){
    int fd=server.AcceptConnect(client.client_info);
    client.client_state=Waiting;
    client_node.push_back(client);
    return fd;
}

int DistributionServer::Write(int sock_fd,char buf[]){
    return server.Write(sock_fd,buf);
}

int DistributionServer::Read(int sock_fd,char buf[]){
    return server.Read(sock_fd,buf);
}

int DistributionServer::Close(int sock_fd){
    auto client_it=find_if(client_node.begin(),client_node.end(),[sock_fd](ClientNode& client){return client.client_info.client_fd==sock_fd;});
    client_node.erase(client_it);
    close(sock_fd);
}