#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
#include <fcntl.h>

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler
    struct sigaction sigaction1;
    sigaction1.sa_flags = SA_RESTART;
    sigaction1.sa_handler = alarmHandler;
    if (sigaction(SIGALRM, &sigaction1, NULL) == -1) {
        perror("smash error: failed to set alarm handler");
    }
    SmallShell& smash = SmallShell::getInstance();
//    std::cout << "smash> ";
    while(true) {
//        std::cout << "smash> "; // TODO: change this (why?)
        std::cout << SmallShell::getInstance().promptName <<"> ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}
