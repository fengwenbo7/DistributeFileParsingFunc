#include "src/distribution_server.h"
#include <sys/epoll.h>
#include <queue>
#include <mutex>

#define MAX_EVENT_NUM 1024

typedef struct{
    int parsing_fd;
}ReadyParsingNode;

DistributionServer workstation_server(5555,10);
DistributionServer fileparsing_server(6666,10);

void addfd_into_epoll(int epoll_fd,int fd,bool et_on=true);
void workstation_handler(int epoll_fd);
void fileparsing_handler();

int main(){
    bool is_workstation_thread_on=false;
    bool is_fileparsing_thread_on=false;
    epoll_event listen_events[MAX_EVENT_NUM];
    int listen_epoll_fd=epoll_create(5);
    //add workstation fd into epoll
    addfd_into_epoll(listen_epoll_fd,workstation_server.GetListenFd());
    //add parsing fd into epoll
    addfd_into_epoll(listen_epoll_fd,fileparsing_server.GetListenFd());

    while(true){
        int event_num=epoll_wait(listen_epoll_fd,listen_events,MAX_EVENT_NUM,-1);
        for(int i=0;i<event_num;i++){
            int sock_fd=listen_events[i].data.fd;
            if(sock_fd==workstation_server.GetListenFd()){
                //workstation connect to this 
                ClientNode workstation_node;
                int workstation_fd=workstation_server.AcceptConnection(workstation_node);
                //add workstation conn_fd into epoll
                int workstation_epoll_fd=epoll_create(10);
                addfd_into_epoll(workstation_epoll_fd,workstation_fd,false);
                //start thread to handle workstation
                if(!is_workstation_thread_on){
                    is_workstation_thread_on=true;
                    thread workstation_t(workstation_handler,workstation_fd);
                    workstation_t.detach();
                }
                cout << "[Connected WorkStation]:\tWorkstation "<< workstation_node.client_info.client_ip << " is connected." << endl;
            }
            else if(sock_fd==fileparsing_server.GetListenFd()){
                //parsing connect to this
                ClientNode parsing_node;
                int parsing_fd=fileparsing_server.AcceptConnection(parsing_node);
                //add parsing conn_fd into epoll
                int parsing_epoll_fd=epoll_create(10);
                addfd_into_epoll(parsing_epoll_fd,parsing_fd,false);
                //start thread to handle file parsing
                if(!is_fileparsing_thread_on){
                    is_fileparsing_thread_on=true;
                    thread fileparsing_t(fileparsing_handler,parsing_fd);
                    fileparsing_t.detach();
                }
                cout << "[Connected Parser]:\tParser " << parsing_node.client_info.client_ip << " is connected and added to queue." << endl;
            }

        }
    }
}

void addfd_into_epoll(int epoll_fd,int fd,bool et_on=true){
    epoll_event event;
    event.data.fd=fd;
    event.events=EPOLLIN;
    if(et_on){
        event.events|=EPOLLET;
    }
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&event);
}

void workstation_handler(int epoll_fd){
    char buf[MAX_BUFFER_SIZE];
    epoll_event events[MAX_EVENT_NUM];
    while(true){
        int event_num=epoll_wait(epoll_fd,events,MAX_EVENT_NUM,-1);
        for(int i=0;i<event_num;i++){
            int sock_fd=events[i].data.fd;
            if(workstation_server.Read(sock_fd,buf)==0){
                //close
                workstation_server.Close(sock_fd);
                //remove epoll
                epoll_ctl(epoll_fd,EPOLL_CTL_DEL,sock_fd,NULL);
                cout << "[Workstation Close]:\tWorkStation client exit! ip:"
                     << workstation_server.GetClientNode(sock_fd).client_info.client_ip
                     << "\tfd:" << workstation_server.GetClientNode(sock_fd).client_info.client_fd
                     << endl;
                continue;
            }
            if(strncmp(buf,"HeartBeats",10)==0){
                //receive heart beats
            }else if(strncmp(buf,"TransmitSuccess",15)==0){
                //receive response of parsing successfully
            }else if(strncmp(buf,"ParsingRequest",14)==0){
                //request from workstation
            }else{
                //cannot be parsed
            }
        }
    }
}

void fileparsing_handler(){

}