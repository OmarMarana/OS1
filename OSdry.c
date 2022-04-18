#include <stdio.h>

int X = 1, p1 = 0, p2 = 0;
int ProcessA() {
    printf("process A\n");
    while(X);
    printf("process A finished\n");
    exit (1);
}
void killAll(){
    if(p2) kill(p2, 15);
    if(p1) kill(p1, 9);
}
int ProcessB() {
    X = 0;
    printf("process B\n");
    killAll();
    printf("process B finished\n");
    return 1;
}
int main(){
    int status;
    if((p1 = fork()) != 0) // if#1
    {
        if((p2 = fork()) != 0){ //if#2
            wait(&status);
            printf("status: %d\n", status);
            wait(&status);
            printf("status: %d\n", status);
        }
        else
        {
            ProcessB();
        }
    }
    else //does this belong to if#1 or if#2?
    {
        ProcessA();
    }
    printf("The end\n");
    return 3;
}