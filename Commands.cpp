#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

#include <dirent.h>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <fcntl.h>
#define PATH_MAX_CP 1000

using namespace std;
const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));



#include <bits/stdc++.h>

//////////////////// TIMEOUT AUX FUNCTIONS //////////////////
int calDiffTime(TimeOutEntry t){
    time_t time_now=time(nullptr);
    int still_time = time_now - t.timestamp;
    return still_time;
}
void setTimer(){
    if(SmallShell::getInstance().timeoutList.empty()){
        alarm(0);///?????????
        return;
    }
    list<TimeOutEntry>::iterator it=SmallShell::getInstance().timeoutList.begin();
    int min_time=it->duration-calDiffTime(*it);
    for(;it!=SmallShell::getInstance().timeoutList.end();++it){
        int diff=calDiffTime(*it);
        min_time=min(it->duration-diff,min_time);
    }
    int time_alarm=max(1,min_time);     // whats this? maybe bcuz alarm takes unsigned int?
    alarm(time_alarm);
}
/////////////////////////////////////////////////////////////
string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

char** getArgs(const char* cmd_line,int* num){
    char** lineArgs = new char *[COMMAND_MAX_ARGS];
    for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
        lineArgs[i] = nullptr;
    }
    *num = _parseCommandLine(cmd_line, lineArgs);
    return lineArgs;
}

//to free args arrays
void cleanUpArray(char** array,int length){ //why use free and delete?
    for(int i=0;i<length;i++){
        free(array[i]);
        array[i]=NULL;
    }
    delete[] array;
    array=NULL;
}

int ConvertCharToInt(const char* str,bool* letter){
    if(strlen(str)==0){ //will not happen
        return -1;
    }
    size_t i=0;
    if(str[0]=='-'){
        i=1;
    }
    for( ;i<strlen(str);i++){
        if(str[i]<'0' || str[i]>'9'){
            *letter=true;
            return -1;
        }
    }
    return atoi(str);
}

int ConvertCharToInt2(const char* str,bool* letter){
    if(strlen(str)==0){ //will not happen
        return -1;
    }
    size_t i=0;
    if(str[0]=='-'){
        i=1;
    }
    for( ;i<strlen(str);i++){
        if(str[i]<'0' || str[i]>'9'){
            *letter=true;
            return -1;
        }
    }
    int num=atoi(str);
    return (num*-1);
}

// TODO: Add your implementation for classes in Commands.h
// TODO: Command Class

//baraah
bool Piped(const char* cmd_line){
    return strchr(cmd_line,'|');
}
//baraah
bool Redirection(const char* cmd_line){
    return strchr(cmd_line,'>');
}

Command::Command(const char *cmd_line) {
    //still commandID + pid not here ?? check
    //length
    length=strlen(cmd_line);
    //commandLine
    commandLine=(char*)malloc(length+1);
    strcpy(commandLine,cmd_line);
    //args
    args=new char*[COMMAND_ARGS_MAX_LENGTH];
    for(int i=0;i<COMMAND_MAX_ARGS;i++){
        args[i]= nullptr;
    }
    length=_parseCommandLine(cmd_line,args);
    //isExternal
    isExternal=false;
    //isBackground
    isBackground=false;
    //isPipe
    isPipe= false;
    isPartOfPipe=false;
    //isRedirection
    isRedirection=false;
}
Command::~Command() {
    //commandLine
    free(commandLine);
    commandLine= nullptr;
    //free the array
    cleanUpArray(args,length);
}

//TODO: BuiltInCommand Class
BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {
    isExternal=false;
}

//TODO: ExternalCommand Class

ExternalCommand::ExternalCommand(const char *cmd_line,JobsList* jobsList):Command(cmd_line) {
    jobs=jobsList;
    isExternal=true;
    //check if is background command
    isBackground=_isBackgroundComamnd(cmd_line);
    // to_execute command
    if(isBackground){
        to_execute[2]= (char*)malloc(strlen(commandLine));
        strcpy(to_execute[2],commandLine);
        _removeBackgroundSign(to_execute[2]);
    }
    else{
        to_execute[2]= (char*)malloc(strlen(commandLine)+1);
        strcpy(to_execute[2],commandLine);
    }
    to_execute[0]=(char*)malloc(10);
    strcpy(to_execute[0],"/bin/bash");
    to_execute[1]=(char*)malloc(3);
    strcpy(to_execute[1],"-c");
    to_execute[3]= nullptr;

    int listSize=SmallShell::getInstance().jobs.jobs.size();
    if(listSize==0){//test change
        commandID=1;
    }else{//test change
        int lastJobId;
        SmallShell::getInstance().jobs.getLastJob(&lastJobId); // now
        commandID=lastJobId+1;//SmallShell::getInstance().jobs.getLastJob(&lastJobId)->externalCommand.commandID+1;
    }

}

void ExternalCommand::execute() { // check ??
    pid=fork();
    if(pid==0){//child
        //execute command
        if(!this->isPartOfPipe) {
            setpgrp();
        }
        execv("/bin/bash",to_execute);
        perror("smash error: execv failed");
        return;
    }
    if(pid>0){ // parent
        if(!isBackground){
            //update current foreGround command
            SmallShell::getInstance().currFgCommand=this;
            //wait
            if(waitpid(pid,NULL,WUNTRACED)==-1){
                perror("smash error: waitpid failed");
            }

            /// What does this mean??????
            SmallShell::getInstance().currFgCommand= nullptr;
            return;
        }
        //add job to list
        //before we add we have to remove finished jobs
        jobs->removeFinishedJobs();
        jobs->addJob(this,false);
        return;

    }
    else{
        //error
        perror("smash error: fork failed");
        return;
    }

}

//TODO: PipeCommand Class
PipeCommand::PipeCommand(const char *cmd_line):Command(cmd_line){
    isPipe=true;
    int lastId;
    SmallShell::getInstance().jobs.removeFinishedJobs();
    SmallShell::getInstance().jobs.getLastJob(&lastId);
    this->commandID=lastId+1;
    // seperate the two commands
    char* command1=(char*)malloc(strlen(cmd_line)+1);
    char* command2=(char*)malloc(strlen(cmd_line)+1);
    bool is_command1=true, is_command2=false;
    size_t i=0,j=0;
    for(i=0;i<strlen(cmd_line);i++) {
        if(is_command1){
            command1[i]=cmd_line[i];
        }
        if(is_command2){
            command2[j]=cmd_line[i];
            j++;
        }
        if (cmd_line[i] == '|') {
            is_command1=false;
            command1[i]='\0';
            is_command2=true;
            sign = (char*)malloc(2);    // mem leak?
            strcpy(sign,"|");
            if (cmd_line[i + 1] == '&') {
                sign = (char*)malloc(3);
                strcpy(sign,"|&");
                i++;
            }
        }
    }
    command2[j]='\0';
    isBackground=_isBackgroundComamnd(cmd_line);
    //if & exist we have to remove from command 2
    if(isBackground){       //why dont we remove the & from cmd1 too?
        _removeBackgroundSign(command2);
    }
    leftCommand=SmallShell::getInstance().CreateCommand(command1);
    leftCommand->isPartOfPipe=true;
    rightCommand=SmallShell::getInstance().CreateCommand(command2);
    rightCommand->isPartOfPipe=true;

}

void PipeCommand::execute() {
    pid_t pid12 = fork();
    if (pid12 == 0) { //son
        setpgrp();
        int fd[2];
        int standard_out0 = dup(0); // stdin is a better name
        int standard_out1 = dup(1);
        int standard_err = dup(2);
        pipe(fd);
        pid_t pid1 = fork();  // why didnt use setpgrp(); here? maybe so that killpg() kills all three processes when killing pipeCmd
        if (pid1 == 0) { // son of son
            if (strcmp(sign , "|")==0) { //sign=="|"
                dup2(fd[1], 1);
                close(fd[0]);
                close(fd[1]);
                //command1
                leftCommand->execute();
                dup2(standard_out1, 1);     // why change the stdout of cmd1 back to its father's stdout?
                close(standard_out1);
            } else {//sign=="|&
                dup2(fd[1], 2);
                close(fd[0]);
                close(fd[1]);
                //command1
                leftCommand->execute();
                dup2(standard_err, 2);
                close(standard_err);
            }
            exit(0);
        }
        if (pid1 == -1) {
            perror("smash error: fork failed");
            close(fd[0]);
            close(fd[1]);

            return;
        }

        pid_t pid2 = fork();
        if (pid2 == 0) {
            if(strcmp(sign , "|")==0) {//sign ="|"
                dup2(fd[0], 0);
                close(fd[0]);
                close(fd[1]);
                //command2
                rightCommand->execute();
                dup2(standard_out0, 0); // why return the stdin of son back to that of the father?
                close(standard_out0);
            } else {//sign== "|&"
                dup2(fd[0], 0);
                close(fd[0]);
                close(fd[1]);
                //command2
                rightCommand->execute();
                dup2(standard_out0, 0);
                close(standard_out0);
            }
            exit(0);
        }
        if (pid2 == -1) {
            perror("smash error: fork failed");
            close(fd[0]);
            close(fd[1]);

            return;
        }
        close(fd[0]);
        close(fd[1]);
        if(waitpid(pid1, nullptr,WUNTRACED)==-1){
            perror("smash error: waitpid failed");
        }
        if(waitpid(pid2, nullptr,WUNTRACED)==-1){
            perror("smash error: waitpid failed");
        }

        exit(0);
    }
    ////////////////////////////////////////////

    if (pid12 == -1) {
        perror("smash error: fork failed");
        return;
    } else {    //parent
        this->pid = pid12;//????  appointing the pid of father (smash) to the pipeCmd??? why? - answer: it's actually the pid of the child that smash forks
        if (!isBackground) {
            SmallShell::getInstance().currFgCommand = this;
            if(waitpid(pid12, nullptr, WUNTRACED)==-1){
                perror("smash error: waitpid failed");
            }
            SmallShell::getInstance().currFgCommand = nullptr;
            return;
        } else {
            SmallShell::getInstance().jobs.removeFinishedJobs();
            SmallShell::getInstance().jobs.addJob(this, false);
        }
    }

}

//TODO JobsList Class
void JobsList::addJob(Command *cmd, bool isStopped) {
//    SmallShell::getInstance().jobs.removeFinishedJobs();//job test
    //make jobEntry
    JobEntry *jobEntry = new JobEntry;
    ////////////
    int size=this->jobs.size();
    if(size==0) {
        jobEntry->externalCommand.commandID = 1;
    }else{
        jobEntry->externalCommand.commandID = cmd->commandID;
    }
    jobEntry->externalCommand.pid = cmd->pid;
    int cmd_len = strlen(cmd->commandLine);
    jobEntry->externalCommand.commandLine = (char *) malloc(cmd_len + 1);//check

    strcpy(jobEntry->externalCommand.commandLine, cmd->commandLine);
    //args?
    jobEntry->externalCommand.isExternal = cmd->isExternal;
    jobEntry->externalCommand.isBackground = cmd->isBackground;
    jobEntry->externalCommand.isPipe = cmd->isPipe;               //not must right?
    jobEntry->externalCommand.isPartOfPipe = cmd->isPartOfPipe;               //not must right?
    jobEntry->externalCommand.isRedirection = cmd->isRedirection; //not must right?
    //finished making jobEntry
    jobEntry->isStopped = isStopped;
    jobEntry->startedTime = time(NULL);
//    jobEntry->externalCommand.islast=true;
    bool insert=false;
    for (list<JobEntry *>::iterator it = jobs.begin(); it != jobs.end(); ++it) {
        if((*it)->externalCommand.commandID>jobEntry->externalCommand.commandID){
            jobs.insert(it,jobEntry);
            insert=true;
            break;
        }
    }
    if(!insert) {
        jobs.push_back(jobEntry);
    }
}

void JobsList::printJobsList() {
    removeFinishedJobs();//TODO:
    for (list<JobEntry *>::iterator it = jobs.begin(); it != jobs.end(); ++it) {
        int jobID = (*it)->externalCommand.commandID;
        char *commandLine = (*it)->externalCommand.commandLine;
        pid_t pid = (*it)->externalCommand.pid;
        time_t currTime = time(NULL);
        time_t diffTime = difftime(currTime, (*it)->startedTime);

        if ((*it)->isStopped) {
            cout<<"["<<jobID<<"] "<<commandLine <<" : "<<pid<<" "<<diffTime<<" secs (stopped)"<<endl;
        } else {
            cout << "[" << jobID << "] " << commandLine << " : " << pid << " " << diffTime << " secs" << endl;
        }
    }
}

void JobsList::removeFinishedJobs() {//baraah check
    list<JobEntry *>::iterator temp;
    for (list<JobEntry *>::iterator it = jobs.begin(); it != jobs.end(); /*++it*/) {
//        (*it)->externalCommand.islast=false; //last
        pid_t res=waitpid((*it)->externalCommand.pid,NULL,WNOHANG);
        if(res!=0){
            //remove
            temp=it;
            ++it;
            /*it=*/jobs.erase(temp);

        }else{
            ++it;
        }
    }
    //update islast
//    JobEntry* last_node= nullptr;
//    for (list<JobEntry *>::iterator it = jobs.begin(); it != jobs.end(); ++it) {
//        last_node=(*it);
//    }
//    if(last_node){
//        last_node->externalCommand.islast=true;
//    }
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
//    SmallShell::getInstance().jobs.removeFinishedJobs();
    for (list<JobEntry *>::iterator it = jobs.begin(); it != jobs.end(); ++it) {
        if(jobId==(*it)->externalCommand.commandID){
            return *it;
        }
    }
    return nullptr;
}

void JobsList::removeJobById(int jobId) {
//    SmallShell::getInstance().jobs.removeFinishedJobs();
    for (list<JobEntry *>::iterator it = jobs.begin(); it != jobs.end(); ++it) {
//        (*it)->externalCommand.islast=false; //last
        if (jobId == (*it)->externalCommand.commandID) {
            jobs.erase(it);
            return;
        }
    }
}

JobsList::JobEntry * JobsList::getLastJob(int *lastJobId) {
//    removeFinishedJobs();
    SmallShell::getInstance().jobs.removeFinishedJobs();
    if(jobs.size()==0){
        return nullptr;
    }
    *lastJobId=(jobs.back())->externalCommand.commandID;
    return jobs.back();
}
/*
JobsList::JobEntry * JobsList::getMaxJob(int *lastJobId) {
//    removeFinishedJobs();
    SmallShell::getInstance().jobs.removeFinishedJobs();
    if(jobs.size()==0){
        return nullptr;
    }
    *lastJobId=0;
    JobEntry* max = nullptr;
    for (list<JobEntry *>::iterator it = jobs.begin(); it != jobs.end(); ++it) {
        if((*it)->externalCommand.commandID > *lastJobId){
            *lastJobId=(*it)->externalCommand.commandID;
            max=(*it);
        }
    }
    return max;
}
*/
JobsList::JobEntry * JobsList::getLastStoppedJob(int *jobId) {
//    SmallShell::getInstance().jobs.removeFinishedJobs();
    JobEntry* to_return= nullptr;
    for (list<JobEntry *>::iterator it = jobs.begin(); it != jobs.end(); ++it) {
        if((*it)->isStopped){
            *jobId=(*it)->externalCommand.commandID;
            to_return= *it;
        }
    }
    return to_return;
}

void JobsList::killAllJobs() {
    SmallShell::getInstance().jobs.removeFinishedJobs();
//    removeFinishedJobs();
    int numOfJobs=jobs.size();
//    if(numOfJobs==0){return;}//test
    cout<<"smash: sending SIGKILL signal to "<<numOfJobs<<" jobs:"<<endl;
    for (list<JobEntry *>::iterator it = jobs.begin(); it != jobs.end(); ++it) {
        pid_t pid=(*it)->externalCommand.pid;
        char* command=(*it)->externalCommand.commandLine;
        cout<<pid<<": "<<command<<endl;
        if((*it)->externalCommand.isPipe){
            if(killpg(pid,SIGKILL)!=0){
                perror("smash error: killpg failed");
                return;
            }
        }else{
            if(kill(pid,SIGKILL)!=0){
                perror("smash error: kill failed");
                return;
            }
        }

    }
}

JobsList::~JobsList() {
    //delete jobs or in ~smallShell?? check
}

//TODO: GetCurrDirCommand Class
GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line){
    //currPath=(char*)malloc(COMMAND_ARGS_MAX_LENGTH);//check ??
    currPath=getcwd(NULL,0);//check ??
}

void GetCurrDirCommand::execute() {
    cout<< currPath <<endl;
}

//TODO: ShowPidCommand Class
ShowPidCommand::ShowPidCommand(const char* cmd_line):BuiltInCommand(cmd_line) {

}

void ShowPidCommand::execute() {
    cout << "smash pid is " << SmallShell::getInstance().smashPid << endl;
    ///// add delete currentcommand
    return;
}

//TODO: JobsCommand Class
JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs) :BuiltInCommand(cmd_line),jobs(jobs){

}

void JobsCommand::execute() {
    jobs->printJobsList();
}

//TODO: ChangeDirCommand Class
ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char** plastPwd):BuiltInCommand(cmd_line),oldPath(plastPwd){

}

void ChangeDirCommand::execute(){
    if(length>2){
        cerr<<"smash error: cd: too many arguments"<<endl ; //cerr cout
        return;
    }
    char* path=getcwd(NULL,0);
    if(args[1]== nullptr)
        return;

    if (strcmp(args[1],"-")==0)
    {
        if(*oldPath == NULL)
        {
            cerr<<"smash error: cd: OLDPWD not set"<<endl ;//cout
            return;
        }
        else {
            if(chdir(*oldPath)== -1){
                perror("smash error: chdir failed"); //test change
//                cout<<"smash error: cd: OLDPWD not set"<<endl ;
                return;
            }
            *oldPath=path;
            return;}
    }
    if(chdir(args[1])== -1){
        perror("smash error: chdir failed"); //test change
//        cout<<"smash error: cd: OLDPWD not set"<<endl ;
        return;
    }
    *oldPath=path;
    return;
}

//TODO: KillCommand Class
bool ValidNumber(const char* ch){
    int length=strlen(ch);
    for(int i=0;i<length;i++){
        if(ch[i]<'0'||ch[i]>'9'){
            return false;
        }

    }
    return true;
};
int powerInt(int x, int y)
{
    for (int i = 0; i < y; i++)
    {
        x *= 10;
    }
    return x;
}
int CharToInt(const char* a)
{

    int sum = 0,len = strlen(a);
    for (int i= 0; i < len; i++)
    {
        int n = a[len - (i + 1)] - '0';
        sum = sum +powerInt(n,i);
    }
    return sum;

}

KillCommand::KillCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line),jobs(jobs){

}
KillCommand::~KillCommand() {}
void KillCommand::execute() {
    if(!this->isPartOfPipe) {
        this->jobs->removeFinishedJobs();
    }
    if(length!=3){
        cerr<<"smash error: kill: invalid arguments"<<endl;//cout
        return;
    }
    char *firstNum =args[1]+1;
    if (*firstNum == '\0' || *args[1]!='-'){
        cerr<<"smash error: kill: invalid arguments"<<endl;//cout
        return;
    }
    bool letter=false; //test change
    int number=ConvertCharToInt(args[2],&letter); //test change
    if(!ValidNumber(firstNum)|| letter){//!ValidNumber(args[2])){
        cerr<<"smash error: kill: invalid arguments"<< endl;//cout
        return;
    }
    JobsList::JobEntry* myList;
    myList=jobs->getJobById(number);//CharToInt(args[2]));
    if(!myList){
        cerr<<"smash error: kill: job-id "<<args[2]<<" does not exist"<<endl;//cout
        return;
    }
    int x=CharToInt(firstNum);
    if(myList->externalCommand.isPipe){    // whats the diff between killpg and kill? and whats the connection with pipe?
        if(killpg(myList->externalCommand.pid,x)==-1){
            perror("smash error: killpg failed");
            return;
        }
    }else{
        if(kill(myList->externalCommand.pid,x)==-1){
            perror("smash error: kill failed");
            return;
        }
    }
//    if(kill(myList->externalCommand.pid,x)==-1){
//        perror("smash error: kill failed");
//        return;
//    }
    cout<<"signal number " << x <<" was sent to pid "<< myList->externalCommand.pid <<endl;

}

//TODO: ForegroundCommand Class
ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobs) :BuiltInCommand(cmd_line),jobs(jobs){
}

void ForegroundCommand::execute() {
    if(!this->isPartOfPipe) {         // whats up with this? seen it more than once
        jobs->removeFinishedJobs();
    }
    //you can minimize the number of lines ******
    //if num of args=2 then get the specified job
    JobsList::JobEntry *jobEntry;
    int jobId;
    if (length == 2) {
        bool letterExist = false;
        jobId = ConvertCharToInt(args[1], &letterExist);
        if (letterExist) {
            cerr << "smash error: fg: invalid arguments" << endl;//cout
            return;
        }

        jobEntry = jobs->getJobById(jobId);
        if (!jobEntry) {
            cerr << "smash error: fg: job-id " << jobId << " does not exist" << endl;//cout
            return;
        }
    } else {
        //if num of args=1 then get last job
        if (length == 1) {
//        int lastJobId;
            jobEntry = jobs->getLastJob(&jobId);
            if (!jobEntry) {
                cerr << "smash error: fg: jobs list is empty" << endl;//cout
                return;
            }

        } else {
            cerr << "smash error: fg: invalid arguments" << endl;//cout
            return;
        }
    }

    pid_t pid1 = jobEntry->externalCommand.pid;
    cout << jobEntry->externalCommand.commandLine << " : " << pid1 << endl;
    if(jobEntry->externalCommand.isPipe){
        if (jobEntry->isStopped&&killpg(pid1, SIGCONT) == -1) {
            perror("smash error: killpg failed");
            return;
        }
    }else{
        if (jobEntry->isStopped && kill(pid1, SIGCONT) == -1) {
            // why if it isSyopped so we kill it ????
            perror("smash error: kill failed");
            return;
        }
    }

    //now command is running foreground
    SmallShell::getInstance().currFgCommand = &(jobEntry->externalCommand);
    jobEntry->isStopped = false;
    SmallShell::getInstance().currFgCommand->isBackground=false;
    waitpid(pid1, NULL, WUNTRACED);
    //finished. now we have to remove from list
    SmallShell::getInstance().currFgCommand = nullptr;
    jobs->removeJobById(jobId);

}

//TODO: BackgroundCommand Class
BackgroundCommand::BackgroundCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line),jobs(jobs){
}
//BackgroundCommand::~BackgroundCommand(){}
void BackgroundCommand::execute(){
    if(length > 2){
        cerr<<"smash error: bg: invalid arguments"<<endl;//cout
        return;
    }
    if(!this->isPartOfPipe) { // whats this?
        jobs->removeFinishedJobs();
    }

    if(length ==1) {
        int ptrInt;
        JobsList::JobEntry *last =jobs->getLastStoppedJob(&ptrInt);
        if (!last) {
            cerr << "smash error: bg: there is no stopped jobs to resume" << endl;//cout
            return;
        }
        pid_t pidlast=last->externalCommand.pid;
        cout<<last->externalCommand.commandLine<<" : "<< pidlast<<endl;
        last->isStopped=false;
        if(last->externalCommand.isPipe){
            if(killpg(last->externalCommand.pid,SIGCONT)==-1)
                perror("smash error: killpg failed");
        }else{
            if(kill(last->externalCommand.pid,SIGCONT)==-1)
                perror("smash error: kill failed");
        }

        return;

    }
    if(length==2) {
        char *temp;
        int jobID = strtol(args[1], &temp, 0);
        if (*temp != '\0') {
            std::cerr << "smash error: bg: invalid arguments" << std::endl;//cout
            return;
        }
        JobsList::JobEntry *job = jobs->getJobById(jobID);
        if (!job || jobID <= 0) {
            std::cerr << "smash error: bg: job-id " << jobID << " does not exist" << std::endl;//cout
            return;
        }
        if (!(job->isStopped)) {
            std::cerr << "smash error: bg: job-id " << jobID << " is already running in the background" << std::endl;//cout
            return;
        }

        if(job->externalCommand.isPipe) {
            if (killpg(job->externalCommand.pid, SIGCONT) == -1) {
                perror("smash error: killpg failed\n");
                return;
            }
        }else{
            if(kill(job->externalCommand.pid, SIGCONT)==-1){
                perror("smash error: kill failed\n");
                return;
            }
        }

        pid_t pidJ=job->externalCommand.pid;
        cout<<job->externalCommand.commandLine  <<" : "<< pidJ <<endl;
        job->isStopped= false;
        return;

    }
    cerr<<"smash error: bg: invalid arguments"<<endl;//cout
    return;
}

//TODO: RedirectionCommand Class
RedirectionCommand::RedirectionCommand(const char *cmd_line):Command(cmd_line) {
    this->isRedirection=true;
    //seperate command </<< filename
    //find first occurence of </<<
    char* just_command_line=(char*)malloc(strlen(cmd_line)+1);
    char* filename_line=(char*)malloc(strlen(cmd_line)+1);
    size_t i,j;
    for( i=0;i<strlen(cmd_line);i++){
        just_command_line[i]=cmd_line[i];
        if(cmd_line[i]=='>'){
            just_command_line[i]='\0';
            sign = (char*)malloc(2);
            strcpy(sign,">");
            i++;
            if(cmd_line[i]=='>'){
                sign = (char*)malloc(3);
                strcpy(sign,">>");
                i++;
            }
            for(j=0;i<strlen(cmd_line);j++,i++){
                filename_line[j]=cmd_line[i];
            }
            filename_line[j]='\0';
            break;
        }
    }
    isBackground=_isBackgroundComamnd(cmd_line);
    if(isBackground){
        //delete &sign
        _removeBackgroundSign(filename_line);
    }
    int numOfArg;
    char **file_args = getArgs(filename_line, &numOfArg); // will not use numOfArgs right???
//    if (numOfArg == 0) {                          ???????
//        return NULL;
//    }
    //now we got the command and file seperated
    this->fileName=(char*)malloc(strlen(filename_line));
    this->fileName=strcpy(this->fileName,file_args[0]);
    seperated_command=SmallShell::getInstance().CreateCommand(just_command_line); // not this line you have to change
    this->seperated_command->isBackground=isBackground;      // shouldn't we ignore & like they said?
    cleanUpArray(file_args,numOfArg);
    // do not forget to check & sign
}

/*RedirectionCommand::~RedirectionCommand(){
    delete(seperated_command);
    free(sign); //??
    sign= nullptr; //??
    free(fileName);//??
    fileName= nullptr;//??
//    Command::~Command();//???
}*/

void RedirectionCommand::execute() {
    ///////////////////////////////////
//    setpgrp();
    int standardOut = dup(1);
    close(1);
    int res;
    if (strcmp(sign,">")==0) {//sign == ">"
        res = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    } else {
        res = open(fileName, O_CREAT | O_WRONLY | O_APPEND, 0666);
    }
    if (res == -1) {
        perror("smash error: open failed");
        dup2(standardOut, 1);
        close(standardOut);
        return;
    }
    seperated_command->execute();
    fflush(stdout);//added
    close(res);//added
    dup2(standardOut, 1);
    close(standardOut);
    return;
    ///////////////////////////////////
    /*
    pid_t pid = fork();
    if (pid == -1) {
        perror("smash error: fork failed");
        return;
    }
    if (pid == 0) {
        setpgrp();
        int standardOut = dup(1);
        close(1);
        int res;
        if (sign == ">") {
            res = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        } else {
            res = open(fileName, O_CREAT | O_WRONLY | O_APPEND, 0666);
        }
        if (res == -1) {
            perror("smash error: open failed");
            return;
        }
        seperated_command->execute();
        dup2(standardOut, 1);
        close(standardOut);

    }
    if(pid>0) {
        if (!isBackground) {
            //update current foreGround command
            SmallShell::getInstance().currFgCommand = this;
            //wait
            if (waitpid(pid, NULL, WUNTRACED) == -1) {//waitpid(pid, NULL, WUNTRACED) == -1) {  ///if(wait(NULL)==-1)
                perror("smash error: wait failed");
                return;
            }
            SmallShell::getInstance().currFgCommand = nullptr;
            return;
        }
    }*/
}

CopyCommand::CopyCommand(const char* cmd_line):BuiltInCommand(cmd_line){ }
CopyCommand:: ~CopyCommand() {
    //  free(commandLine);

}
void CopyCommand::execute() {
    //printf("command is %s and isBackG %d\n",commandLine,isBackground);
    isBackground=_isBackgroundComamnd(commandLine);
    char *src = strdup(args[1]);
    char *dst = strdup(args[2]);
    char file_path_orgign[PATH_MAX_CP+1];
    char file_path_output[PATH_MAX_CP+1];
    if(_isBackgroundComamnd(dst))
        _removeBackgroundSign(dst);
    if(dst[strlen(dst)-1]==' '){
        dst[strlen(dst)-1]='\0';
    }
    void* input1=realpath(src,file_path_orgign);
    realpath(dst,file_path_output);


    int input, LOLO;
    if((strcmp(file_path_output,file_path_orgign)==0)&&(input1!=NULL)) {
        cout << "smash: " << src << " was copied to " << dst << "" << endl;
        fflush(stdout);
        return;

    }
    input = open(src, O_RDONLY);
    if (input < 0) {
        perror("smash error: open failed");
        return;
    }
    LOLO = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (LOLO < 0) {
        perror("smash error: open failed");
        if (close(input) == -1) {
            perror("smash error: close failed");

        }
        return;
    }
    pid = fork();
    if (pid == 0) {//child
        //execute command
        setpgrp();

        //cout << "smash: " << args[1] << " and " << args[2] << endl;
        std::array<char, 1024> buf;
        bool Done = true;
        while (true) {
            int readnum = (int) read(input, buf.data(), 1024);
            if (readnum == -1) {
                perror("smash error: read failed");
                Done = false;
                break;
            }
            if (readnum == 0)
                break;

            int writecount = (int) write(LOLO, buf.data(), readnum);
            if (writecount == -1) {
                perror("smash error: write failed");
                Done = false;
                break;
            }
            // if(readnum != writecount)
            //{
            //perror("smash error: copy failed");
            //}
        }


        if (Done) {
            cout << "smash: " << args[1] << " was copied to " << args[2] << endl;

        }
        if (close(LOLO) == -1) {
            perror("smash error: close failed");
            exit(1);
            return;
        }

        if (close(input) == -1) {
            perror("smash error: close failed");

        }
        //cout<<"LOLO COPY ENDED"<<endl;
        exit(1);
        return;

    }
    if(pid>0){//parent
        /*
        if(isPartOfTimeout){
            time_t t_now = time(nullptr);
            TimeOutEntry new_time(pid,this->duration,t_now,this->timeCommandLine);
            SmallShell::getInstance().timeoutList.push_front(new_time);
            setTimer();
        }*/
        if(!isBackground){
//            int status;
//            int wpid;
            //update current foreGround command
            SmallShell::getInstance().currFgCommand=this;
            //wait
            waitpid(pid,NULL,WUNTRACED);
            //while((wpid=wait(&status))!=-1);
            //{
            //     perror("smash error: waitpid failed");
            //}
            SmallShell::getInstance().currFgCommand= nullptr;
            return;
        }
        //add job to list
        //before we add we have to remove finished jobs
        SmallShell::getInstance().jobs.removeFinishedJobs();
        SmallShell::getInstance().jobs.addJob(this,false); //??
        return;

    }
    else{
        //error
        perror("smash error: fork failed");
        return;
    }

}

HeadCommand::HeadCommand(const char* cmd_line):BuiltInCommand(cmd_line){ }
HeadCommand::~HeadCommand(){}
void HeadCommand::execute() { // we have to fork?? should we support ctrl-Z??
    int N=10;
    bool flag=false;
    char* curr_file;
    if(length==1){//no arguments
        cerr << "smash error: head: not enough arguments" << endl;
        return;
    }
    if(length>3){//no arguments
        cerr << "smash error: head: too many arguments" << endl;
        return;
    }
    if (length==2){ // check if only number
        curr_file = strdup(args[1]);
    }
    else {
        if(length == 3){
            curr_file = strdup(args[2]);
            N= ConvertCharToInt2(args[1],&flag);
            if (flag){
                cerr << "smash error: head: invalid arguments" << endl;
                return;
            }
        }
    }
    int curr_file_fd = open(curr_file, O_RDONLY);
    if (curr_file_fd < 0) {
       perror("smash error: open failed");
       return;
    }
   // int i;
    std::array<char, 1> buf;
    bool line;
    while(N>0) {
        line = true;
       // i = 0;
        while (line) {
            int readnum = (int) read(curr_file_fd, buf.data(), 1);
            if (readnum == -1) {
                perror("smash error: read failed");
                break;
            }
            if (readnum == 0)
                break;
            if (buf[0] == '\n') {
                line = false;
            }
            int writecount = (int) write(STDOUT_FILENO, buf.data(), readnum);
            if (writecount == -1) {
                perror("smash error: write failed");
                break;
            }
            //i++;
        }
        N--;
    }
    if (close(curr_file_fd) == -1) {
        perror("smash error: close failed");
    }
    //return;
}

//TODO: QuitCommand
QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs) :BuiltInCommand(cmd_line),jobs(jobs) {
}
void QuitCommand::execute() {
    if( (length>1) && ( strcmp(args[1],"kill\0")==0) ){
        jobs->killAllJobs();
    }
//    what exit(0) do?????
    exit(0);
}

TailCommand::TailCommand(const char *cmd_line, int N) : BuiltInCommand(cmd_line), N(N)
{
    ///////////////////////////////
    //check the isPartOfPipe condition
    ///////////////////////////////


    if(length !=2 && length != 3 )
    {
        cerr<<"smash error: tail: invalid arguments"<<endl;//cout
        return;
    }

    if(length == 3 && (args[1][0] != '-' ||args[1][1] == '\0'  ||!ValidNumber(args[1]+1))
    {
        cerr<<"smash error: tail: invalid arguments"<<endl;//cout
        return;
    }

    // remove & from filename
    if(length == 2)
    {
        // N should be 10 by default
        _removeBackgroundSign(args[1]);
        filename = std::string(args[1]);
    }
    else
    {
        this.N = CharToInt(args[1][1]);
        _removeBackgroundSign(args[2]);
        filename = std::string(args[2]);
    }

}

void TailCommand::execute()
{
    std::ifstream in(filename);

    if (in.is_open())
    {
        std::vector<std::string> lines_in_reverse;
        std::string line;
        while (std::getline(in, line))
        {
            // Store the lines in reverse order.
            lines_in_reverse.insert(lines_in_reverse.begin(), line);
        }
        int i = 0;
//        int lines_num = 1;
//        int res;
//        res = open("C:\\Users\\omarm\\Desktop\\test\\myout.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
//        dup2(res,1);
        for(const auto& value: lines_in_reverse) {
            if(i == this->N)
            {
                break;
            }
            std::string str = value + "\n";
            write(1,str.c_str() , str.length());
//            std::cout << value << "\n";

            i++;
        }
    }
    else
    {
        perror("smash error: open failed");
        return;
    }



}

ChpromptCommand::ChpromptCommand(const char* cmd_line):Command(cmd_line){

    if(length==1){
        if(this->promptName){
            free(this->promptName);
            this->promptName= nullptr;
        }
        //smash
        this->promptName= nullptr;
        this->promptName=(char*)malloc(5+1);
        strcpy(this->promptName ,"smash");
    }
    if(length>1 && args[1]) {
        if(this->promptName){
            free(this->promptName);
            this->promptName= nullptr;
        }
        int len=strlen(args[1]);
        this->promptName=(char*)malloc(len+1);
        strcpy(this->promptName ,args[1]);
    }
}

void ChpromptCommand::execute() {
    if(promptName) {
        free(SmallShell::getInstance().promptName);
        SmallShell::getInstance().promptName=(char*)malloc(strlen(promptName)+1);       //SmallShell::getInstance().promptName isnt this just lvalue? does it actually change the member?
        strcpy(SmallShell::getInstance().promptName,this->promptName);
    }
}
//TODO: SmallShell Class
SmallShell::SmallShell() {
// TODO: add your implementation
    smashPid=getpid();
    nextIndex=1;
    currFgCommand= nullptr;
    plastPwd= nullptr;
//    promptName="smash";
    this->promptName=(char*)malloc(5+1);
    strcpy(this->promptName ,"smash");
}

SmallShell::~SmallShell() {
// TODO: add your implementation
    delete currFgCommand;
    free(plastPwd);
    plastPwd= nullptr;
    free(promptName);
    promptName= nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////
lsCommand::lsCommand(const char *cmd_line):BuiltInCommand(cmd_line) {

}

void lsCommand::execute() {
    struct dirent **namelist;
    int num;
    char* path=getcwd(NULL,0);
    num=scandir(path,&namelist,NULL,alphasort);
    if(num < 0){
        perror("smash error:scandir failed");
        return;
//        exit(-1); // return ??? ask
    }else{
        for(int i=0;i<num;i++){//change
            cout<<namelist[i]->d_name<<endl;
            free(namelist[i]);
        }
//        num--;
//        while(num>=0){
//            //cout<<(“%s\n”,namelist[n]->d_name)<<endl;
//            cout<<namelist[num]->d_name<<endl;
//            free(namelist[num]);
//            num--;
//        }
        free(namelist);
    }
    //exit(0);
}
////////////////////////////////////////////////////////////////////////////////////////////////
//// TIMEOUT COMMAND

bool isBuiltInCommand(char* comm){
    return strcmp(comm,"chprompt")==0 || strcmp(comm, "showpid") == 0 || strcmp(comm, "pwd") == 0 ||
           strcmp(comm, "ls") == 0 || strcmp(comm, "cd") == 0 || strcmp(comm, "jobs") == 0 ||
           strcmp(comm, "kill") == 0 || strcmp(comm, "fg") == 0 ||
           strcmp(comm, "bg") == 0 || strcmp(comm, "quit") == 0 || strcmp(comm, "cp") == 0; //ask

}

TimeOutCommand::TimeOutCommand(const char *cmd_line) :Command(cmd_line){
    //to check valid arguments?????
    if(length<3){
        return;
    }
    this->isTimeoutCommand=true;
    this->isBuiltIn=isBuiltInCommand(this->args[2]);

    //make the new command without timeout+number
    char* new_command=(char*)malloc(strlen(cmd_line)+1);
    int i=0;
    for(; !((*(cmd_line + i)) >= '0' && (*(cmd_line + i)) <= '9' && (*(cmd_line + i + 1)) == ' ') ; i++){  // why ! in " !((*(cmd_line + i))"?
    }
    strcpy(new_command,cmd_line+i+2);
    newCommand=SmallShell::getInstance().CreateCommand(new_command);

    newCommand->isTimeoutCommand=true;
    int time=CharToInt(this->args[1]);
    newCommand->duration=time;
    this->duration=time;
    newCommand->timeCommandLine=(char*)malloc(strlen(cmd_line)+1);
    strcpy(newCommand->timeCommandLine,cmd_line);
}

void TimeOutCommand::execute() {
//    int duration=CharToInt(this->args[1]);
    bool isNotNumber= false;
    if(this->length<3 || *(this->args[1])=='-' ){
        cerr << "smash error: timeout: invalid arguments" << endl;//cout
        return;
    }
    ConvertCharToInt(this->args[1],&isNotNumber);
    if(isNotNumber){
        cerr << "smash error: timeout: invalid arguments" << endl;//cout
        return;
    }
    if(this->isBuiltIn){
        time_t t_now = time(nullptr);
        char* ch=(char*)malloc(2);//added
        strcpy(ch,"-");//added
        TimeOutEntry tEntry(-1,duration, t_now, ch);
        SmallShell::getInstance().timeoutList.push_front(tEntry);
        setTimer(); // I think we shouldn't do this since it sends a signal to smash itself in the case of bulit in cmds
        this->newCommand->execute();
        free(ch);//added
        return;
    }
    //cp+external
    // check if external/cp so create external then execute

//    newCommand->execute();
    //external code
    pid=fork();
    ExternalCommand* externalCommand= dynamic_cast<ExternalCommand *>(newCommand);
    this->commandID=externalCommand->commandID;//we have to update so it will be added to jobList okay
    if(pid==0){//child
        //execute command
        if(!this->isPartOfPipe) {
            setpgrp();
        }
        execv("/bin/bash",externalCommand->to_execute);
        perror("smash error: execv failed");
        return;
    }
    if(pid>0) {//parent
//        if(this->isTimeoutCommand){
        time_t t_now = time(nullptr);
        TimeOutEntry new_time(pid, this->duration, t_now, this->commandLine);//???
        SmallShell::getInstance().timeoutList.push_front(new_time);
        setTimer();
//        }
//        pid=pid1;
        if (!externalCommand->isBackground) {
            //update current foreGround command
            SmallShell::getInstance().currFgCommand = this;
            //wait
            if (waitpid(pid, NULL, WUNTRACED) == -1) {
                perror("smash error: waitpid failed");
            }
            SmallShell::getInstance().currFgCommand = nullptr;
            return;
        }
        //add job to list
        //before we add we have to remove finished jobs
        SmallShell::getInstance().jobs.removeFinishedJobs();
        SmallShell::getInstance().jobs.addJob(this, false); //??
        return;

    }
    else{
        //error
        perror("smash error: fork failed");
        return;
    }

}
////////////////////////////////////////////////////////////////////////////////////////////////
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

    int numOfArg;
    char **lineArgs = getArgs(cmd_line, &numOfArg);
    if (numOfArg == 0) {
        return NULL;
    }
    //timeout

    if (strcmp(lineArgs[0], "timeout\0") == 0) {  //DONE
        cleanUpArray(lineArgs, numOfArg);
        return new TimeOutCommand(cmd_line);
    }

    if(Piped(cmd_line)){
        cleanUpArray(lineArgs,numOfArg);
        return new PipeCommand(cmd_line);
    }

    if(Redirection(cmd_line)){
        cleanUpArray(lineArgs,numOfArg);
        return new RedirectionCommand(cmd_line);
    }

    /*1*/
    if (strcmp(lineArgs[0],"chprompt\0") == 0 || strcmp(lineArgs[0],"chprompt&\0") == 0) { //test changed
        cleanUpArray(lineArgs,numOfArg);
        return new ChpromptCommand(cmd_line);
    }

    /*2*/
    if (strcmp(lineArgs[0],"showpid\0") == 0 || strcmp(lineArgs[0],"showpid&\0") == 0) { //test changed
        cleanUpArray(lineArgs,numOfArg);
        return new ShowPidCommand(cmd_line);
    }

    /*3*/
    if (strcmp(lineArgs[0], "pwd\0") == 0) {  //DONE
        cleanUpArray(lineArgs, numOfArg);
        return new GetCurrDirCommand(cmd_line);
    }

    /*4*/
    if (strcmp(lineArgs[0],"cd\0") == 0) {
        cleanUpArray(lineArgs,numOfArg);
        return new ChangeDirCommand(cmd_line, &plastPwd);
    }

    /*5*/
    if (strcmp(lineArgs[0],"jobs\0") == 0) {
        cleanUpArray(lineArgs,numOfArg);
        return new JobsCommand(cmd_line, &jobs);
    }

    /*6*/
    if (strcmp(lineArgs[0],"kill\0") == 0) {
        cleanUpArray(lineArgs,numOfArg);
        return new KillCommand(cmd_line, &jobs);
    }

    /*7*/
    if (strcmp(lineArgs[0],"fg\0") == 0) {
        cleanUpArray(lineArgs,numOfArg);
        return new ForegroundCommand(cmd_line, &jobs);
    }
    /*8*/
    if (strcmp(lineArgs[0],"bg\0") == 0) {
        cleanUpArray(lineArgs,numOfArg);
        return new BackgroundCommand(cmd_line, &jobs);
    }
    //tail
    if (strcmp(lineArgs[0],"tail\0") == 0) {
        cleanUpArray(lineArgs,numOfArg);
        return new TailCommand(cmd_line);
    }


    if (strcmp(lineArgs[0],"head\0") == 0) {
        cleanUpArray(lineArgs,numOfArg);
        return new HeadCommand(cmd_line);
    }
    /*9*/
    if (strcmp(lineArgs[0],"quit\0") == 0) {
        cleanUpArray(lineArgs,numOfArg);
        return new QuitCommand(cmd_line, &jobs);
    } else {
        cleanUpArray(lineArgs,numOfArg);
        return new ExternalCommand(cmd_line,&jobs); //changed ?? baraah
    }


    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    /*
    int numOfArg;
    char **lineArgs = getArgs(cmd_line, &numOfArg);
    if (numOfArg == 0) {
        std::cout << "smash> ";
        return ;
    }
    if (strcmp(lineArgs[0], "chprompt\0") == 0 || strcmp(lineArgs[0], "chprompt&\0") == 0 ) { //DONE
        if (numOfArg >= 2) {
            std::cout << lineArgs[1] << "> ";
        } else {
            std::cout << "smash> ";
        }
        cleanUpArray(lineArgs, numOfArg);
        return ;
    }
*/
    Command* cmd = CreateCommand(cmd_line);
    if(!cmd){
//         std::cout << "smash> ";//baraah
        return;
    }
    cmd->execute();

    delete (cmd);//???
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}



