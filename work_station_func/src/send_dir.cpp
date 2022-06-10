#include "send_dir.h"

bool SendDirectory::setDir(string path){
    if((dp=opendir(path.c_str()))==nullptr){
        return false;
    }
    dir_path=path;
    return true;
}

bool SendDirectory::sendFile(Server* s,int fd,const char* dname){
    char buf[1024];
    string file_name(dname);
    string file_path=dir_path+"/"+string(file_name);

    ifstream in_file(file_path.c_str());
    if(in_file.is_open()){
        in_file.seekg(0,in_file.end);
        int file_size=in_file.tellg();
        in_file.seekg(0,in_file.beg);

        in_file.read(buf,8);
        buf[8]='\0';

        stringstream send_buf;
        send_buf<<file_name<<":"<<buf<<";"<<file_size;
        strcpy(buf,send_buf.str().c_str());

        s->Write(fd,buf);
    }
}

bool SendDirectory::sendDir(Server* s,int fd){
    while((dirp=readdir(dp))!=nullptr){
        if(strncmp(dirp->d_name,".",1)==0||strncmp(dirp->d_name,"..",2)==0)
            continue;
        sendFile(s,fd,dirp->d_name);
    }
    s->Write(fd,"SendDirFinished");
    closedir(dp);
}