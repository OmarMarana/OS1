#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <list>
#include <cstring>
//#include <string.h>

using namespace std;
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)
#define COMMAND_MAX_LEN (80)

class Command {
// TODO: Add your data members
public:
    int commandID;
    pid_t pid;
    char* commandLine;
    int length;//num_args
    char** args;
    bool isExternal;
    bool isBackground;
    bool isPipe;
    bool isPartOfPipe;
    bool isRedirection;
    bool isTimeoutCommand;
    int duration;
    char* timeCommandLine;
    bool islast;

    Command(){} //baraah ask
    Command(Command& comm):commandID(comm.commandID),pid(comm.pid),length(comm.length),args(nullptr),
                           isExternal(comm.isExternal),isBackground(comm.isBackground),isPipe(comm.isPipe),isPartOfPipe(comm.isPartOfPipe)
            ,isRedirection(comm.isRedirection),duration(comm.duration){
        this->commandLine=(char*)malloc(length+1);
        strcpy(this->commandLine,comm.commandLine);
    }
    Command(const char* cmd_line);
    virtual ~Command();
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line);
    virtual ~BuiltInCommand() =default;
};

class JobsList;//changed line ?? baraah
class ExternalCommand : public Command {
public:
    JobsList* jobs; //baraah ??
    char* to_execute[4]; //
    ExternalCommand(){}  //baraah ask
    ExternalCommand(Command* comm):Command(*comm){//added
    }
    ExternalCommand(const char* cmd_line,JobsList* jobsList);
    virtual ~ExternalCommand() {
        for(int i=0;i<3;i++){
            free(to_execute[i]);
        }
    }
    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    Command* leftCommand;
    char* sign;
    Command* rightCommand;
    PipeCommand(const char* cmd_line);
    virtual ~PipeCommand(){
        free(sign);
        sign= nullptr;
    }
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    Command* seperated_command;
    char* sign;
    char* fileName;
    explicit RedirectionCommand(const char* cmd_line);
    virtual ~RedirectionCommand(){}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};


class TailCommand : public BuiltInCommand {
public:
    int N;
    std::string filename;
    TailCommand(const char* cmd_line,int N =10);
    virtual ~TailCommand() {}
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    char** oldPath;
    ChangeDirCommand(const char* cmd_line, char** plastPwd);
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
    char* currPath; // baraah
public:
    GetCurrDirCommand(const char* cmd_line);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line);
    virtual ~ShowPidCommand() {}
    void execute() override;
};

//class JobsList; //changed line ?? baraah
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    JobsList* jobs;
    QuitCommand(const char* cmd_line, JobsList* jobs);
    virtual ~QuitCommand() =default;
    void execute() override;
};

class CommandsHistory {  //return
protected:
    class CommandHistoryEntry {
        // TODO: Add your data members
        char* commandLine;
        //sequence needed **** ?? ask
    };
    // TODO: Add your data members

public:
    CommandsHistory();
    ~CommandsHistory() {}
    void addRecord(const char* cmd_line);
    void printHistory();
};

class HistoryCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    HistoryCommand(const char* cmd_line, CommandsHistory* history);
    virtual ~HistoryCommand() {}
    void execute() override;
};

class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
    public:
        ExternalCommand externalCommand; //ask baraah
        bool isStopped;
        time_t startedTime;
    };
    // TODO: Add your data members
    list<JobEntry*> jobs; //ask ???
//  list<JobEntry*> stoppedJobs; //to delete baraah ??
public:
    JobsList(){} //changed
    ~JobsList(); //check
    void addJob(Command* cmd, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry * getLastJob(int* lastJobId);
//    JobEntry * getMaxJob(int* lastJobId);//added job test baraah ask
    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobs;
public:
    JobsCommand(const char* cmd_line, JobsList* jobs);
    virtual ~JobsCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobs;
public:
    KillCommand(const char* cmd_line, JobsList* jobs);
    virtual ~KillCommand() ;
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobs;
public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobs;
public:
    BackgroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~BackgroundCommand()=default;
    void execute() override;
};


// TODO: should it really inhirit from BuiltInCommand ?
class CopyCommand : public BuiltInCommand {
public:
    CopyCommand(const char* cmd_line);
    virtual ~CopyCommand() ;
    void execute() override;
};

class HeadCommand: public BuiltInCommand{
public:
    HeadCommand(const char* cmd_line);
    virtual ~HeadCommand();
    void execute() override;
};


// TODO: add more classes if needed
// maybe chprompt , timeout ?
class ChpromptCommand: public Command{
public:
    char* promptName;
    ChpromptCommand(const char* cmd_line);
    void execute();    // why not override?
};

class lsCommand: public BuiltInCommand{
public:
    lsCommand(const char* cmd_line);
    virtual ~lsCommand() {}
    void execute() override;
};

class TimeOutEntry{
public:
    pid_t pid;
    int duration;
    int timestamp;
    char cmd_line[COMMAND_MAX_LEN];
    TimeOutEntry()=default;
    TimeOutEntry(pid_t pid, int duration,int timestamp,char* cmd_line)
            :pid(pid),duration(duration),timestamp(timestamp){
        strcpy(this->cmd_line, cmd_line);
    }
};
/*
class TimeList{
public:
    list<TimeOutObject> timeList;
    TimeList()=default;
    ~TimeList()=default;
};
*/

class TimeOutCommand: public Command{
public:
    bool isBuiltIn;
    Command* newCommand;
    TimeOutCommand(const char* cmd_line);
    virtual ~TimeOutCommand() {}
    void execute() override;
};

class SmallShell {
// private:
    // TODO: Add your data members
public:
    JobsList jobs;
    list<TimeOutEntry> timeoutList;
    int nextIndex; //delete ??
    Command* currFgCommand; //update it ?? baraah
    char* plastPwd
    pid_t smashPid;
    char* promptName;
    // TODO: History
    // we have to save directory ?? check
    SmallShell();
public:
    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
