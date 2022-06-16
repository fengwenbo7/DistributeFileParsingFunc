#include "webserver.h"
#include "string.h"
#include <iostream>

using namespace std;

WebServer::WebServer(
            int port, TrigMode trigMode, int timeoutMS, bool OptLinger,
            int sqlPort, const char* sqlUser, const  char* sqlPwd,
            const char* dbName, int connPoolNum, int threadNum):
            port_(port),openLinger_(OptLinger),timeoutMS_(timeoutMS),isClose_(false)
{
    //set resources path
    srcDir_=getcwd(nullptr,256);
    assert(srcDir_);
    strncat(srcDir_,"resources",16);
    HttpConn::userCount=0;
    HttpConn::srcDir=srcDir_;
    //todo:sql_conn

    InitEventMode_(trigMode);
    if(InitSocket_()){
        std::cout<<"========== Server init =========="<<std::endl;
        std::cout<<"Port:"<<port_<<", OpenLinger: "<<(int)OptLinger<<std::endl;
        std::cout<<"Listen Mode: "<<(listen_event_ & EPOLLET ? "ET": "LT")<<", OpenConn Mode: :"<<(conn_event_ & EPOLLET ? "ET": "LT")<<std::endl;
        std::cout<<"srcDir: "<< HttpConn::srcDir<<std::endl;
        std::cout<<"SqlConnPool num: "<<connPoolNum<<", ThreadPool num: "<< threadNum<<std::endl;
    }
    else{
        isClose_=true;
        std::cout<<"========== Server init error!=========="<<std::endl;
    }
}

void WebServer::InitEventMode_(TrigMode trigMode) {
    listen_event_=EPOLLRDHUP;//tcp connection closed by the opposite
    conn_event_=EPOLLONESHOT|EPOLLRDHUP;//oneshot should be reactivated when receiving it
   switch (trigMode)
    {
    case TrigMode::kNoneET:
        break;
    case TrigMode::kConnET:
        conn_event_ |= EPOLLET;
        break;
    case TrigMode::kListenET:
        listen_event_ |= EPOLLET;
        break;
    case TrigMode::kAllET:
        listen_event_ |= EPOLLET;
        conn_event_ |= EPOLLET;
        break;
    default:
        listen_event_ |= EPOLLET;
        conn_event_ |= EPOLLET;
        break;
    }
    HttpConn::isET=(conn_event_&EPOLLET);
}

bool WebServer::InitSocket_(){
    int ret;
    //initial sockaddr
    struct sockaddr_in addr;
    if(port_>65535||port_<1024){
        std::cout<<"port:"<<port_<<" error"<<std::endl;
        return false;
    }
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(port_);

    //socket
    listenFd_=socket(AF_INET,SOCK_STREAM,0);
    if(listenFd_<0){
        std::cout<<"socket error"<<std::endl;
        return false;
    }

    //set socket
    struct linger optLinger = { 0 };
    if(openLinger_) {
        /* 优雅关闭: 直到所剩数据发送完毕或超时 */
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }
    ret=setsockopt(listenFd_,SOL_SOCKET,SO_LINGER,&optLinger,sizeof(optLinger));
    if(ret<0){
        close(listenFd_);
        std::cout<<"init linger error"<<std::endl;
        return false;
    }

    int optval = 1;
    /* 端口复用 */
    /* 只有最后一个套接字会正常接收数据。 */
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        std::cout<<"set socket setsockopt error !"<<std::endl;
        close(listenFd_);
        return false;
    }

    //bind
    ret=bind(listenFd_,(const sockaddr*)&addr,sizeof(addr));
    if(ret<0){
        std::cout<<"bind error !"<<std::endl;
        close(listenFd_);
        return false;
    }

    //listen 
    ret=listen(listenFd_,5);
    if(ret<0){
        std::cout<<"listen error !"<<std::endl;
        close(listenFd_);
        return false;
    }

    //add listen_fd to epoll
    ret=epoller_->AddFd(listenFd_,listen_event_|EPOLLIN);
    if(ret==0){
        std::cout<<"epoll add error !"<<std::endl;
        close(listenFd_);
        return false;
    }

    //non block
    SetFdNonblock(listenFd_);
    std::cout<<"port:"<<port_<<" success."<<std::endl;
    return true;
}

int WebServer::SetFdNonblock(int fd){
    assert(fd>0);
    return fcntl(fd,F_SETFL,fcntl(fd,F_GETFL)|O_NONBLOCK);
}

void WebServer::Start(){
    int timeMS=-1;//no event blocked
    if(!isClose_){
        std::cout<<"server start..."<<std::endl;
    }
    while(!isClose_){
        int eventnum=epoller_->Wait(timeMS);
        for(int i=0;i<eventnum;i++){
            int sock_fd=epoller_->GetEventFd(i);
            uint32_t events=epoller_->GetEvents(i);
            if(sock_fd==listenFd_){
                //new client connect
                DealListen_();
            }
            else if(events&(EPOLLRDHUP|EPOLLHUP|EPOLLERR)){
                //client disconnect
                assert(users_.count(sock_fd)>0);
                DealCloseConn_(&users_[sock_fd]);
            }
            else if(events&EPOLLIN){
                //deal read
                assert(users_.count(sock_fd)>0);
                DealRead_(&users_[sock_fd]);
            }
            else if(events&EPOLLOUT){
                //deal write
                assert(users_.count(sock_fd)>0);
                DealWrite_(&users_[sock_fd]);
            }
            else{
                std::cout<<"unexpected event."<<std::endl;
            }
        }
    }
}

void WebServer::DealListen_(){
    struct sockaddr_in addr;
    socklen_t len=sizeof(addr);
    int conn_fd=accept(listenFd_,(struct sockaddr*)&addr,&len);
    if(conn_fd<0){
        std::cout<<"accept error"<<std::endl;
        close(conn_fd);
        return;
    }
    if(HttpConn::userCount>=MAX_FD){
        std::cout<<"too many users"<<std::endl;
        close(conn_fd);
        return;
    }
    users_[conn_fd].init(conn_fd,addr);
    epoller_->AddFd(conn_fd,EPOLLIN|conn_event_);
    SetFdNonblock(conn_fd);
    std::cout<<"client:"<<users_[conn_fd].GetFd()<<" connect.";
}

void WebServer::DealCloseConn_(HttpConn* client){
    assert(client);
    epoller_->DelFd(client->GetFd());
    client->Close();
    std::cout<<"client:"<<client->GetFd()<<" disconnect.";
}

void WebServer::DealRead_(HttpConn* client) {
    assert(client);
    threadpool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
}

void WebServer::DealWrite_(HttpConn* client) {
    assert(client);
    threadpool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

void WebServer::OnRead_(HttpConn* client){
    assert(client);
    int read_errno=0;
    int ret=client->read(&read_errno);
    if(ret<=0&&read_errno!=EAGAIN){
        std::cout<<"client:"<<client->GetFd()<<" read error:"<<read_errno<<".it should be closed."<<std::endl;
        DealCloseConn_(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnWrite_(HttpConn* client){
    assert(client);
    int write_errno=0;
    int ret=client->write(&write_errno);
}

void WebServer::OnProcess(HttpConn* client){
    //conn_fd is set one-shot,so it should be reseted
    if(client->process()){
        epoller_->ModFd(client->GetFd(),EPOLLOUT|conn_event_);
    }
    else{
        epoller_->ModFd(client->GetFd(),EPOLLIN|conn_event_);
    }
}

