#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_VALUE 1000
#define MIN_VALUE 2
#define BUF_SIZE 100


/*
Write a program that creates N number of child processes. The N is given as a command line argument and should be between 1 and 10.

Each child process will generate random numbers in a loop until the number generated is prime. 
When this happens it sends back to the parent process a message containing its PID and the prime number. Then it exits.
The parent process creates all the child processes and then waits for the messages. Each time it receives a message, it prints out it content. 
After it received the expected number of messages it proceeds to wait on all the child processes in order to not create orphans and then it terminates.

To send the messages back to the parent process, use a pipe. The child processes will inherit the file descriptors and they will be able to send data back.
You can find details of this technique in The Linux Programming Interface.
*/

int isPrime(int n){
    int i;
    if(n % 2 == 0){
        return 0;
    }
    for(i = 3; i <= sqrt(n); i = i + 2){
        if(n % i == 0){
            return 0;
        }
    }
    return 1;
}



int main(int argc, char *argv[])
{
    int pfd[2];                             /* Process synchronization pipe */
    int j;
   
    char buf[BUF_SIZE];                     //buffer to use for reading;
    char buf_write[BUF_SIZE];               //buffer to use for writing;   
    ssize_t numRead;                        //how much to read
    int status = 0;
    pid_t wpid;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        printf("Usage: %s <number_of_childs>", argv[0]);
    setbuf(stdout, NULL);                   /* Make stdout unbuffered, since we
                                               terminate child with _exit() */
    printf("Parent started\n");

    if (pipe(pfd) == -1){
        printf("Creating pipe failed!\n");
        exit(EXIT_FAILURE);
    }
    int num;
    num = strtol(argv[1], NULL, 10);
    for (j = 1; j <= num; j++) {

        switch (fork()) {
        case -1:
            exit(EXIT_FAILURE);
        case 0: /* Child */
            if (close(pfd[0]) == -1){        /* Read end is unused */
                printf("Child closing read end failed!\n");
                exit(EXIT_FAILURE);
            }
            
            srand(time(NULL) - j*2);
            //sleep(3);
            int t;
            do{
                t = (rand() % (MAX_VALUE+1-MIN_VALUE)) + MIN_VALUE;
            }while(!isPrime(t));
            //print the value
            printf("Child %d (PID=%ld) generated the prime number: %d\n", j, (long) getpid(), t); 
            //now write it on the pipe
            snprintf(buf_write, BUF_SIZE, "%ld - %d", (long) getpid(), t);
            printf ("This is buf_write: %s\n", buf_write);
            if (write(pfd[1], buf_write, (strlen(buf_write)+1))){
                //printf("Child writing on the pipe failed!\n");
                //printf("Error code is %d\n", errno);
                //printf("%s\n", strerror(errno));
                //exit(EXIT_FAILURE);
            }


            /*printf("Child %d (PID=%ld) closing pipe\n",
                     j, (long) getpid());*/

            if (close(pfd[1]) == -1)
                exit(EXIT_FAILURE);

            _exit(EXIT_SUCCESS);
        default: /* Parent loops to create next child */
            break;
        }
    }
    /* Parent comes here; close write end of pipe so we can see EOF */

    if (close(pfd[1]) == -1){               /* Write end is unused */
        
        printf("Parent closing write end failed!\n");
        exit(EXIT_FAILURE);
    }                
        
    

    for (;;) {              /* Read data from pipe, echo on stdout */
        numRead = read(pfd[0], buf, BUF_SIZE);
        if (numRead == -1)
            exit(EXIT_FAILURE);
        if (numRead == 0)
            break;                      /* End-of-file */
        printf("Parent process writing...\n");
        if (write(STDOUT_FILENO, buf, numRead) != numRead){
            printf("Error writing to STDOUT_FILENO");
            exit(EXIT_FAILURE);
        }
        printf("\nParent process end writing...\n");  
    }
    
    while ((wpid = wait(&status)) > 0);    
    printf("Parent ready to go\n");
    
    exit(EXIT_SUCCESS);
}