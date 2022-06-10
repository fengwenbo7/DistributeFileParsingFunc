#include "../src_units/client.h"
#include "../src_units/server.h"
#include "src/send_dir.h"
#include <thread>

Client distri_client("127.0.0.1",5555);
SpecificTime st;

void HeartBeathandler_t();

int main(){
    Server parsing_server(8888,1);
    while(distri_client.Connect()!=0){
        sleep(1);
    }
    thread heartbeats_t(HeartBeathandler_t);
    heartbeats_t.detach();

    parsing_server.Listen();

    while(true){
        CLientInfo parsing_info;
        SendDirectory sd;
        //get preview info of file
        cout<<"please enter the directory you want to send:\t";
        string dir_path;
        while(true){
            cin>>dir_path;
            if(sd.setDir(dir_path)==true){
                break;
            }
            else{
                cout<<"directory path is illegal,please enter again:\t";
            }
        }


    }
}

void HeartBeathandler_t(){
    char buf[MAX_BUFFER_SIZE];
    while(true){
        if(distri_client.Read(buf)==0){
            distri_client.Close();
            cout<<"["<<st.getTime().c_str()<<"distribute closed]:\tdisconnected from distributer server."<<endl;
            exit(0);
        }
        if(strncmp(buf,"HeartBeats",10)==0){
            //receive heart beat package
            strcpy(buf,"HeadtBeats");
            distri_client.Write(buf);//reply heart beats
        }else{
            cout<<"["<<st.getTime().c_str()<<" Reply Error]:\tWorkStation Server cannot parse the reply information from dsitribute server:"<<buf<<endl;
        }
    }
}