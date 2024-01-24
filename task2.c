    // Task2 of RTS warsaw course, the dining philosophers problem

    #include <stdio.h>
    #include <stdlib.h>
    #include <pthread.h>
    #include <unistd.h>
    #include <time.h>
    #include <sys/types.h>
    #include <sys/ipc.h>
    #include <sys/sem.h>
    #include <sys/wait.h>
    #include <sys/shm.h>
    #include <signal.h>
    #define phil_Num 5 //number of philosophers
    #define THINKING 0 //philosopher is thinking
    #define HUNGRY 1 //philosopher wants to eat but has no fork
    #define EATING 2 //philosopher is eating
    #define fork_Num 5 //number of forks

    void grab_fork( int left_fork_id );

    void put_away_fork( int left_fork_id );

    void philosopher(int id);

    void SIGINT_handler(int signum);

    void print_semaphore_values();
    
    struct shm{
        int state[phil_Num];
        int hungry_count;
    };
    struct shm *shared_memory;

    void Initialize_Shared_Memory() {
        int shmid = shmget(IPC_PRIVATE, sizeof(struct shm), 0666 | IPC_CREAT);
        
        if (shmid == -1) {
            perror("shmget");
            exit(EXIT_FAILURE);
        }

        printf("Memory attached at shmid %d\n", shmid);

        shared_memory = (struct shm *)shmat(shmid, NULL, 0);

        if (shared_memory == (struct shm *)(-1)) {
            perror("shmat");
            exit(EXIT_FAILURE);
        }
    }
    int max_state2_count = 0;
    int semaphore_id;
    pid_t pid_num[phil_Num];

int main(){
        //initializing shared memory
        Initialize_Shared_Memory();

        //initializing semaphores
        semaphore_id = semget(IPC_PRIVATE, fork_Num, 0666 | IPC_CREAT );
        for(int i=0; i< fork_Num;i++){
            semctl(semaphore_id, i, SETVAL, 1);
            printf("Semaphore %d initialized value %d \n", i, semctl(semaphore_id, i, GETVAL, 0));
        }

        // Create philosopher processes
        pid_t pid;
        for(int i = 0; i < phil_Num; i++){
           // sleep(rand() % 2 +1);
            pid = fork();
            pid_num[i] = pid;
            if (pid == 0) {
                printf("Philosopher %d initialized\n", i);
                philosopher(i);
                exit(0);
            }
            if(pid < 0){
                printf("Error creating process\n");
                exit(1);
            }
        }
    
    // Parent process prints each philosopher's state
    if(pid > 0){ //parent process
        signal(SIGINT, SIGINT_handler);
        int state1_count,state2_count;
        
        while(1){
            state1_count = 0;
            state2_count = 0;
            for (int i = 0; i < phil_Num; i++)
            {
                printf("Philosopher [%d] is %d\n", (i+1), shared_memory->state[i]);
                if(shared_memory->state[i] == 1){
                    state1_count++;
                }
                if (shared_memory->state[i] == 2)
                {
                    state2_count++;
                }
                
            }

            printf("\n");
            if (state2_count > max_state2_count)
            {
                max_state2_count = state2_count;
            }
            
            sleep(1);
        }
    }

    return 0;
    }

    void grab_fork( int left_fork_id ){
        struct sembuf sem_op[2];

        sem_op[0].sem_num = left_fork_id;
        sem_op[0].sem_op = -1;
        sem_op[1].sem_num = (left_fork_id + 1) % fork_Num;
        sem_op[1].sem_op = -1;
        semop(semaphore_id, sem_op, 2);

    }

    void put_away_fork(int left_fork_id){
        struct sembuf sem_op[2];

        sem_op[0].sem_num = left_fork_id;
        sem_op[0].sem_op = 1;
        sem_op[1].sem_num = (left_fork_id + 1) % fork_Num;
        sem_op[1].sem_op = 1;
        semop(semaphore_id, sem_op, 2);

    }

void philosopher(int id) {
    while(1){
        shared_memory->state[id] = 0; //Thinking
        usleep(rand() % 500000); 
            
        shared_memory->state[id] = 1;

        grab_fork(id); 
        shared_memory->hungry_count++;
//      usleep(rand() % 500000);     

        shared_memory->state[id] = 2; //Eating
        shared_memory->hungry_count--;
        usleep(rand() % 500000);     

        put_away_fork(id);

    }
}

void SIGINT_handler(int signum){
        printf("\nSIGINT received, cleaning up and exiting\n");
        // Cleanup: remove semaphores
        for (int i = 0; i < fork_Num; i++) {
            semctl(semaphore_id, i, IPC_RMID, 0);
        }
        for(int i= 0; i< phil_Num; i++){
            kill(pid_num[i], SIGKILL);
        }
        shmdt(shared_memory);
        printf("Max number of philosophers eating at the same time: %d\n", max_state2_count);
        exit(0);
}

void print_semaphore_values(){
    for(int i=0; i< fork_Num;i++){
        printf("Semaphore %d value %d \n", i, semctl(semaphore_id, i, GETVAL, 0));
    }
}
