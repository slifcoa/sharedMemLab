#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdbool.h>

#define SIZE 4096
#define MAX 50

void sigHandler(int);
//int running = 1;

struct sharedMemory{
    int running;
    int flag;
    int turn;
    char input[MAX];

};

//bool readLine = false;

int main(){

    key_t key;
    int shmId;
    char *pathname = "/tmp";
    struct sharedMemory *shmPtr;
    bool readLine = false;
    //Generates an IPC key, both programs need identical key
    key = ftok(pathname, 4);
    if(key == -1){
    printf("ftok failed with errno = %d\n", errno);
    return -1;
    }

    //Get's a specified region for shared memory and grants necessary permissions 
    //for the specific program
    if ( (shmId = shmget(key,sizeof(struct sharedMemory) * MAX, 0666)) < 0) {
        perror("Error creating shared memory.\n");
        exit(1);
    }

     //Attaches the specified region of shared memory to the address
    //space of the calling process
    if ( (shmPtr = (struct sharedMemory *)shmat(shmId, 0, 0)) == (void*) -1) {
        perror("Can't attach shared memory to address.\n");
        exit(1);
    }
    //Handle Graceful Shutdowns Accordingly
   // signal(SIGINT, sigHandler);
   printf("Reader Program starting...\n"); 
    
    //While loop, will terminate when writer gracefully shut's down
    while(shmPtr[0].running == 1){
    
    //Reader busy wait's since either he has read the line or writer's writing
    while(shmPtr[0].flag == 2 || readLine){
	if(shmPtr[0].flag == 0){
	    readLine = false;
	}
    }

    //readLine = false;
    printf("%s", shmPtr[0].input);
    readLine = true;
    shmPtr[0].flag++;
    }
    //Detaches the shared memory region upon shutting down
    if (shmdt (shmPtr) < 0) {
    perror ("just can't let go\n");
    exit (1);
    }

    return 0;
}
