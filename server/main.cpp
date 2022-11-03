#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <memory.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <vector>
#include <sys/mman.h>

#define MAX_EVENT_NUM 1024
#define BUFFER_SIZE 1024
#define PORT 7239

struct fds
{
    int epoll_fd;
    int sock_fd;
};

void add_fd_into_epoll_fds(int epollfd,int tarfd,bool oneshot){
    //add fd to epoll
    epoll_event event;
    event.data.fd=tarfd;
    event.events=EPOLLIN|EPOLLET;
    if(oneshot){
        event.events|=EPOLLONESHOT;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,tarfd,&event);
    //set non-block
    int old_option=fcntl(tarfd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(tarfd,F_SETFL,new_option);
}

//epoll events shot only once because we set the oneshot attribute,so we should set again after EAGAIN occurs
void reset_epolloneshot(int epoll_fd,int conn_fd){
    epoll_event event;
    event.data.fd=conn_fd;
    event.events=EPOLLIN|EPOLLET|EPOLLONESHOT;
    epoll_ctl(epoll_fd,EPOLL_CTL_MOD,conn_fd,&event);
}

std::vector<std::string> getFiles(std::string cate_dir)
{
	std::vector<std::string> files;//存放文件名

	DIR *dir;
	struct dirent *ptr;
	char base[1000];
 
	if ((dir=opendir(cate_dir.c_str())) == NULL)
        {
		perror("Open dir error...");
                exit(1);
        }
 
	while ((ptr=readdir(dir)) != NULL)
	{
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0||((std::string)ptr->d_name).find(".txt")==std::string::npos)    ///current dir OR parrent dir
		        continue;
		else if(ptr->d_type == 8)    ///file
			//printf("d_name:%s/%s\n",basePath,ptr->d_name);
			files.push_back(ptr->d_name);
		else if(ptr->d_type == 10)    ///link file
			//printf("d_name:%s/%s\n",basePath,ptr->d_name);
			continue;
	}
	closedir(dir);
	return files;
}

void* worker(fds* fds){
    int conn_fd=fds->sock_fd;
    int epoll_fd=fds->epoll_fd;
    pthread_t pid=pthread_self();
    char buf[BUFFER_SIZE];
    memset(buf,'\0',BUFFER_SIZE);
    while(1){
        int ret=recv(conn_fd,buf,BUFFER_SIZE-1,0);
        printf("receive ret:%d\n",ret);
        if(ret==0){//close connection
            printf("client closed.\n");
            epoll_ctl(epoll_fd,EPOLL_CTL_DEL,conn_fd,NULL);//remove current socket from epoll
            close(conn_fd);
            break;
        }
        else if(ret<0){
            if(errno==EAGAIN){//can register again,not internet error
                printf("read later.\n");
                reset_epolloneshot(epoll_fd,conn_fd);
                break;
            }
            else{//error occurs
                perror("recv error\n");
                break;
            }
        }
        else{
            printf("[thread:%d] get messgae:%s\n",(int)pid,buf);
            const std::string& str(buf);
            if(str.find("FILE_LIST")!=std::string::npos){
                DIR *dir;
                char basePath[100];
                ///get the current absoulte path
                memset(basePath, '\0', sizeof(basePath));
                getcwd(basePath, 999);
                printf("the current dir is : %s\n",basePath);
                std::vector<std::string> files=getFiles(basePath);
                //send to client
                for(auto iter:files){
                    send(conn_fd,iter.c_str(),sizeof(iter),0);
                }
            }else if(str.find("FILE_LOAD_")!=std::string::npos){
                int index=str.find_last_of('_');
                std::string file_name=str.substr(index+1);
                std::cout<<"load file "<<file_name<<std::endl;

                // 打开文件并读取文件数据
                FILE *fp = fopen(file_name.c_str(), "r");
                if(NULL != fp){
                    char buffer[BUFFER_SIZE];
                    bzero(buffer, BUFFER_SIZE);
                    int length = 0;
                    // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止
                    while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
                    {
                        if(send(conn_fd, buffer, length, 0) < 0)
                        {
                            printf("Send File:%s Failed./n", file_name.c_str());
                            break;
                        }
                        bzero(buffer, BUFFER_SIZE);
                    }
 
                    // 关闭文件
                    fclose(fp);
                    printf("File:%s Transfer Successful!\n", file_name.c_str());
                }
            }else{
                std::cout<<"client send message:"<<str<<std::endl;
            }
        }
    }
    printf("end thread receiving data on fd:%d",conn_fd);
    return NULL;
}

int main(){
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        perror("socket error\n");
        return -1;
    }

    struct sockaddr_in listen_addr;
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_addr.s_addr=INADDR_ANY;
    listen_addr.sin_port=htons(PORT);
    int ret=bind(listen_fd,(const sockaddr*)&listen_addr,sizeof(listen_addr));
    if(ret==-1){
        perror("bind error\n");
        return -1;
    }

    ret=listen(listen_fd,5);
    if(ret==-1){
        perror("listen error\n");
        return -1;
    }

    epoll_event events[MAX_EVENT_NUM];
    int epoll_fd=epoll_create(5);
    if(epoll_fd<0){
        perror("epoll create error\n");
        return -1;
    }

    add_fd_into_epoll_fds(epoll_fd,listen_fd,false);

    while(1){
        ret=epoll_wait(epoll_fd,events,MAX_EVENT_NUM,-1);
        if(ret<0){
            perror("epoll wait error\n");
            break;
        }
        for(int i=0;i<ret;i++){
            int sock_fd=events[i].data.fd;
            if(sock_fd==listen_fd){
                struct sockaddr_in client_addr;
                socklen_t len=sizeof(client_addr);
                int conn_fd=accept(listen_fd,(sockaddr*)&client_addr,&len);
                add_fd_into_epoll_fds(epoll_fd,conn_fd,true);
                printf("client connect\n");
            }
            else if(events[i].events&EPOLLIN){
                printf("epoll in occurs\n");
                fds fd_new_process;
                fd_new_process.epoll_fd=epoll_fd;
                fd_new_process.sock_fd=sock_fd;
                int pid=fork();
                if(pid==0){
                    //child process
                    close(listen_fd);
                    worker(&fd_new_process);
                }
            }
            else{
                printf("something else happened\n");
            }
        }
    }
    close(listen_fd);
    return 0;
}