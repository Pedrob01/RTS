// main function gets NUM_CHILD from argv[1] and creates NUM_CHILD child processes with fork() and sleep(1) between each fork()

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#define NUM_CHILD 11
int SIGINT_press = 0;
#define WITH_SIGNALS

#ifdef WITH_SIGNALS
//Signal handler, if one child process has an error being created, SIGTERM is sent to all other child processes, exits with code 1
void sig_handler_parent(int signo) {
    // Ignore all signals and restore the default signal handler for SIGCHLD
    if(signo == SIGCHLD){
        signal(SIGCHLD, SIG_DFL);
    }
    else{
        signal(signo, SIG_IGN);
    }
}
void sig_handler_SIGINT(int signo){
    SIGINT_press = 1;
}

void sig_handler_SIGTERM(int signo){
    //if(signo == SIGTERM){
        printf("\nchild[%d] : Received SIGTERM process will be terminated\n", getpid());
    //}
}
#endif

//error message handler
int error_msg(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int n_child_exit = 0;
    #ifdef WITH_SIGNALS
    pid_t child_pid[NUM_CHILD]; // Used for saving child pid so when SIGINT is pressed all children get the signal
    // Set the signal handler for all signals to ignore them
    for(int j=1; j<=64;j++){
        if(j == SIGINT){signal(SIGINT, sig_handler_SIGINT);}
        else{signal(j,sig_handler_parent);}
    }
    #endif

    for(int i= 0; i<NUM_CHILD; i++){
        pid_t pid=fork();
        
        if (pid == 0)
        {   
            #ifdef WITH_SIGNALS
            signal(SIGINT, SIG_IGN);           
            signal(SIGTERM, sig_handler_SIGTERM);
            #endif

            printf("\nchild[%d] Parent identifier: [%d] , Sleeping for 10 seconds \n",getpid(), getppid());
            sleep(10);
            printf("\nchild[%d] Executed child process\n",getpid());
            exit(0);
        }
        else if(pid > 0){// parent process
                    sleep(1);
            #ifdef WITH_SIGNALS
            child_pid[i] = pid;
            if(SIGINT_press == 1){
               
                printf("\nparent[%d] SIGINT received, sending children SIGTERM\n",getpid());
                for(int j=0; j<=i; j++){
                    kill(child_pid[j], SIGTERM);
                }
                break;
            }
            #endif 
        }
        else{
            #ifdef WITH_SIGNALS
            signal(SIGTERM, SIG_DFL);
            #endif
            error_msg("Error forking");
        }
    }

    printf("\nparent[%d] Finished creating child processes\n",getpid());

    while(wait(NULL) > 0){
        n_child_exit++;
    }

    printf("\nAll children exited, %d, children are no more\n", n_child_exit);
        
    #ifdef WITH_SIGNALS
    //restores all signals to default
    for(int i = 0; i < 64; i++) {
    signal(i, SIG_DFL);
    }
    #endif
    return 0;
}
