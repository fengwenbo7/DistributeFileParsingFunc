#include "file_parsing.h"

FileParsing::FileParsing(char* ip,int port):parsing_client(ip,port){
    waiting_state=new WaitingState();
    parsing_state=new ParsingState();
}

void FileParsing::Init(){
    SpecificTime st;
    while(parsing_client.Connect()==-1){
        sleep(1);
    }
    cout << "[" << st.getTime().c_str() << " Connected]:\tconnected to task distributer server at port 6666" << endl;
    SetState(waiting_state);
}

inline void FileParsing::SetState(State * s) {
    state = s;
}

void FileParsing::Handler() {
    state->Handler(this);
}

WaitingState* FileParsing::GetWaitingState(){
    return waiting_state;
}

ParsingState* FileParsing::GetParsingState(){
    return parsing_state;
}

Client& FileParsing::GetClient(){
    return this->parsing_client;
}

FileParsing::~FileParsing(){
    this->parsing_client.Close();
}

void WaitingState::Handler(FileParsing* fp){
    char buf[MAX_BUFFER_SIZE];
    SpecificTime st;
    cout << "[" << st.getTime().c_str() << " Waiting Tasks]:\tWating for tasks from task distributer server!" << endl;
    while(true){
        
    }
}

void ParsingState::Handler(FileParsing* fp){

}