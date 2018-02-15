#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#define SIZE 4096
#define MAX 50

void sigHandler (int);

//int running = 1;

struct sharedMemory{

    int running;
    int flag;
    int turn;
    char input[MAX];

} ;

struct sharedMemory *shmPtr ;

int main()
{

    key_t key;
    int shmId;
    char *pathname = "/tmp";
    struct sharedMemory sm = {1, 2,0, " "};
    
    //Generates an IPC key, both programs need identical key
    key = ftok(pathname, 4);
    if(key == -1){
    printf("ftok failed with errno = %d\n", errno);
    return -1;
    }

    //Get's a specified region for shared memory and grants necessary permissions 
    //for the specific program
    if ( (shmId = shmget(key, sizeof(struct sharedMemory)*MAX, 0666|IPC_CREAT)) < 0) {
        perror("Error creating shared memory.\n");
        exit(1);
    }

    //Attaches the specified region of shared memory to the address
    //space of the calling process
    if ( (shmPtr = (struct sharedMemory *)shmat(shmId, 0, 0)) == (void*) -1) {
        perror("Can't attach shared memory to address.\n");
        exit(1);
    }
    memcpy(&shmPtr[0], &sm, sizeof(struct sharedMemory));
    //Handle Graceful Shutdowns Accordingly
    signal(SIGINT, sigHandler);
    printf("%d", shmPtr[0].running);
    //loop that handles user input
    
    printf("Writer Program Starting...\n");
    while(shmPtr[0].running  == 1){
   
    //Busy wait for writer to sit  
    while(shmPtr[0].flag != 2); 

    fgets(shmPtr[0].input, MAX, stdin);

    //Breaks loop if CTRL-C'ed
    if(shmPtr[0].running == 0){
    break;
    }

    printf("%s\n", shmPtr[0].input);
    shmPtr[0].flag = 0;
    }

    //Detaches the shared memory region upon shutting down
    if (shmdt (shmPtr) < 0) { 
    perror ("just can't let go\n"); 
    exit (1); 
    } 

    //free's the shared space used by the program
    if (shmctl (shmId, IPC_RMID, 0) < 0) { 
      perror ("can't deallocate\n"); 
      exit(1); 
    }
    return 0;
}

//Handle signal interrupts
void sigHandler (int sigNum){
    if(sigNum == SIGINT){
        shmPtr[0].running = 0;
	shmPtr[0].flag = 0;
    }
}
