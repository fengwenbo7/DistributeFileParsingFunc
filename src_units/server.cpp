#include "server.h"

Server::Server(int port,int size):listen_port(port),listen_size(size){
    this->Init();
    this->Listen();
}

void Server::Init(){
    listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd==-1){
        SpecificTime st;
        fprintf(stderr,"[%s Socket Error]:\t%s \n\a",st.getTime().c_str(),strerror(errno));
        exit(1);
    }
    //bind
    bzero(&server_addr,sizeof(struct sockaddr_in));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htons(INADDR_ANY);
    server_addr.sin_port=htons(listen_port); 

    if(bind(listen_fd,(const sockaddr*)&server_addr,sizeof(server_addr))==-1){
        SpecificTime st;
        fprintf(stderr,"[%s Bind Error]:\t%s \n\a",st.getTime().c_str(),strerror(errno));
        exit(1);
    }
}

void Server::Listen(){
    if(listen(listen_fd,listen_size)==-1){
        SpecificTime st;
        fprintf(stderr,"[%s Listen Error]:\t%s \n\a",st.getTime().c_str(),strerror(errno));
        exit(1);
    }
}

int Server::GetListenFd(){
    return this->listen_fd;
}

int Server::AcceptConnect(CLientInfo& info){
    socklen_t len=sizeof(info.client_addr);
    info.client_fd = accept(listen_fd,(struct sockaddr*)&info.client_addr,&len);
    if(info.client_fd==-1){
        SpecificTime st;
        fprintf(stderr,"[%s Accept Error]:\t%s \n\a",st.getTime().c_str(),strerror(errno));
        exit(1);
    }
    info.client_ip.assign(inet_ntoa(info.client_addr.sin_addr));
    return info.client_fd;
}

int Server::Write(int sock_fd,char buf[]){
    int nbytes = 0;
    if((nbytes = write(sock_fd, buf, strlen(buf))) == -1) {
        SpecificTime st;
        fprintf(stderr, "[%s Write Error]:\t%s\n", st.getTime().c_str(), strerror(errno));
    }
    return nbytes;
}

int Server::Read(int sock_fd,char buf[]){
    int nbytes = 0;
    if ((nbytes = read(sock_fd, buf, MAX_BUFFER_SIZE)) == -1) {
        SpecificTime st;
        fprintf(stderr, "[%s Read Error]:\t%s\n", st.getTime().c_str(), strerror(errno));
    } else
        buf[nbytes] = '\0';

    return nbytes;
}