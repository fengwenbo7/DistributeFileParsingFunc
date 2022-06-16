/*
fengwenbo 2022-06
*/

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <auto_ptr.h>
#include <memory>

#include "epoller.h"
#include "../http/http_conn.h"
#include "../pool/thread_pool.h"

enum class TrigMode{
    kNoneET=0,
    kConnET=1,
    kListenET=2,
    kAllET=3,
};

class WebServer{
public:
    WebServer(
        int port,TrigMode trigMode,int timeoutMS,bool optlinger,
        int sqlport,const char* sqluser,const char* sqlpwd,
        const char* dbname,int connpoolnum,int threadnum
    );
    ~WebServer();
    void Start();
    
private:
    bool InitSocket_(); 
    void InitEventMode_(TrigMode trigMode);

    void DealListen_();
    void DealCloseConn_(HttpConn* client);
    void DealWrite_(HttpConn* client);
    void DealRead_(HttpConn* client);
    void OnRead_(HttpConn* client);
    void OnWrite_(HttpConn* client);
    void OnProcess(HttpConn* client);

    static const int MAX_FD = 65536;
    static int SetFdNonblock(int fd);

private:
    int port_;
    bool openLinger_;
    int timeoutMS_;  /* 毫秒MS */
    bool isClose_;
    int listenFd_;
    char* srcDir_;

    uint32_t listen_event_;
    uint32_t conn_event_;

    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int,HttpConn> users_;//<fd,http_conn>
};

#endif