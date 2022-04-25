#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <sys/wait.h>
#include <unistd.h>

//#include <ctime>

using namespace std;

void ctrlZHandler(int sig_num) {
    // TODO: Add your implementation
    cout<<"smash: got ctrl-Z"<<endl;
    Command* currCommand=SmallShell::getInstance().currFgCommand;
    if(!currCommand){
        return;
    }
    //if command is background what happen ???
//    if(!currCommand->isPartOfPipe)//tarek
//        SmallShell::getInstance().jobs.removeFinishedJobs();//?????????????
    SmallShell::getInstance().jobs.addJob(currCommand, true);//?? check
    pid_t pid=currCommand->pid;
//    if(killpg(pid,SIGSTOP)==-1) {//check ???????????????
//        perror("smash error: kill failed");
//        return;
//    }
    if(currCommand->isPipe){
        if(killpg(pid,SIGSTOP)==-1) {//check ???????????????
            perror("smash error: killpg failed");
            return;
        }
    }else{
        if(kill(pid,SIGSTOP)==-1){//check ???????????????
            perror("smash error: kill failed");
            return;
        }}
    SmallShell::getInstance().currFgCommand= nullptr;
    cout<<"smash: process "<<pid<<" was stopped"<<endl;
}
//////////////////////////////////////////////////
void ctrlCHandler(int sig_num) {
    // TODO: Add your implementation
    cout<<"smash: got ctrl-C"<<endl;
    Command* currCommand=SmallShell::getInstance().currFgCommand;
    if(!currCommand){
        return;
    }
    // if(currCommand->isBackground){//check ?? don't need right??
    //   return;
    //}
    pid_t pid=currCommand->pid;
    //delete
//    cout<<"Signal.cpp: the pid is"<<pid << endl;//to delete
//    cout<<"Signal.cpp: the pgrp is" << endl;//to delete

//    if (killpg(pid, SIGKILL) == -1) {//test changed
//        perror("smash error: kill failed");
//        return;
//    }
    if(currCommand->isPipe) {
        if (killpg(pid, SIGKILL) == -1) {//test changed
            perror("smash error: killpg failed");
            return;
        }
    }else{
        if (kill(pid, SIGKILL) == -1) {//test changed
            perror("smash error: kill failed");
            return;
        }
    }
    SmallShell::getInstance().jobs.removeJobById(pid);
    SmallShell::getInstance().currFgCommand= nullptr;
    cout<<"smash: process "<<pid<<" was killed"<<endl;
}
//////////////////////////////////////////////////
int cal(TimeOutEntry t){
    time_t time_now=time(nullptr);
    int still_time = time_now - t.timestamp;
    return still_time;
}
//int min(int a,int b){
//    if(a<=b){
//        return a;
//    }
//    return b;
//}

void ChangeSetAlarm(){
    if(SmallShell::getInstance().timeoutList.empty()){
        alarm(0);///?????????
        return;
    }
    list<TimeOutEntry>::iterator it=SmallShell::getInstance().timeoutList.begin();
    int min_time=it->duration-cal(*it);
    for(;it!=SmallShell::getInstance().timeoutList.end();++it){
        int diff=cal(*it);
        min_time=min(it->duration-diff,min_time);
    }
    int time_alarm=max(1,min_time);
    alarm(time_alarm);

}
void alarmHandler(int sig_num) {
    // TODO: Add your implementation

    cout<<"smash: got an alarm"<<endl;

    list<TimeOutEntry>::iterator temp;
    list<TimeOutEntry>::iterator it=SmallShell::getInstance().timeoutList.begin();
    for( ; it != SmallShell::getInstance().timeoutList.end();){
        if(cal(*it) >= it->duration){//tokill
            int res=waitpid(it->pid,NULL,WNOHANG);
            if(res==0) {
                if (kill(it->pid, SIGKILL) == 0) {
                    int cmp = strcmp("-", it->cmd_line);
                    if (cmp != 0) {
                        cout << "smash: " << it->cmd_line << " timed out!" << endl;
                    }
                } else {
                    perror("smash error: kill failed");
                }
            }
            temp=it;
            ++it;
            SmallShell::getInstance().timeoutList.erase(temp);

        }else{
            ++it;
        }
    }
    ChangeSetAlarm();
}

/*
    timeout 5 sleep 100&
    timeout 10 sleep 200&
    timeout 2 sleep 300&
    timeout 7 sleep 400&
    jobs
 */