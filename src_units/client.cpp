#include "client.h"

Client::Client(char* server_ip,int server_port){
    strncpy(this->server_ip,server_ip,16);
    this->server_port=server_port;

    struct hostent *host;
    if((host=gethostbyname(server_ip))==NULL){
        SpecificTime st;
        fprintf(stderr, "[%s Host Error]:\tThe host name %s is illegal.\n", st.getTime().c_str(), server_ip);
        exit(1);
    }

    if((this->sock_fd=socket(AF_INET,SOCK_STREAM,0))==-1){
        SpecificTime st;
        fprintf(stderr, "[%s Socket Error]:\tInit socket fd error!\n", st.getTime().c_str());
        exit(1);
    }

    bzero(&client_addr,sizeof(client_addr));
    client_addr.sin_family=AF_INET;
    client_addr.sin_port=htons(server_port);
    client_addr.sin_addr=*((struct in_addr*)host->h_addr);
}

int Client::Connect(){
    int ret;
    if((ret=connect(sock_fd,(const struct sockaddr*)&client_addr,sizeof(client_addr)))==-1){
        SpecificTime st;
        fprintf(stderr, "[%s Connect Error]:\tconnect socket fd error!\n", st.getTime().c_str());
        exit(1);
    }
    return ret;
}

char* Client::GetServerIP(){
    return server_ip;
}

int Client::GetServerPort(){
    return server_port;
}

int Client::Write(char* buf){
    int ret;
    if((ret=write(sock_fd,buf,sizeof(buf)))==-1){
        SpecificTime st;
        fprintf(stderr, "[%s Write Error]:\t%s\n", st.getTime().c_str(), strerror(errno));
    }
    return ret;
}

int Client::Read(char* buf){
    int ret;
    if((ret=read(sock_fd,buf,MAX_BUFFER_SIZE))==-1){
        SpecificTime st;
        fprintf(stderr, "[%s Read Error]:\t%s\n", st.getTime().c_str(), strerror(errno));
    }else{
        buf[ret]='\0';
    }
    return ret;
}

int Client::Close(){
    close(this->sock_fd);
}