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
            }
            else if(events&(EPOLLRDHUP|EPOLLHUP|EPOLLERR)){
                //client disconnect
            }
            else if(events&EPOLLIN){
                //deal read
            }
            else if(events&EPOLLOUT){
                //deal write
            }
            else{
                std::cout<<"unexpected event."<<std::endl;
            }
        }
    }
}

