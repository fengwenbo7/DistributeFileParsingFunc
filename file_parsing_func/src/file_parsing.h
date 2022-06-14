#ifndef DISTRIBUTEDFILEPARSING_FILE_PARSING_H
#define DISTRIBUTEDFILEPARSING_FILE_PARSING_H

#include "../../src_units/client.h"

class FileParsing;

//base class for state
class State{
public:
    virtual void Handler(FileParsing* fp)=0;
    virtual ~State()=default;
};

//state of waiting 
class WaitingState:public State{
public:
    void Handler(FileParsing* fp);
    ~WaitingState()=default;
};

//state of file parsing 
class ParsingState:public State{
public:
    void Handler(FileParsing* fp);
    ~ParsingState()=default;
};

//class for file parsing with state mode 
class FileParsing{
private:
    Client parsing_client;
    State* state;
    WaitingState* waiting_state;
    ParsingState* parsing_state;
public:
    FileParsing(char* ip,int port=6666);
    void Init();
    void Handler();
    inline void SetState(State* state);
    WaitingState* GetWaitingState();
    ParsingState* GetParsingState();
    Client& GetClient();
    ~FileParsing();
};

#endif