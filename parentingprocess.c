#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_VALUE 1000
#define MIN_VALUE 2


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
    int j, dummy;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        printf("Usage: %s <number_of_childs>", argv[0]);
    setbuf(stdout, NULL);                   /* Make stdout unbuffered, since we
                                               terminate child with _exit() */
    printf("Parent started\n");

    if (pipe(pfd) == -1)
        exit(EXIT_FAILURE);
    int num;
    num = strtol(argv[1], NULL, 10);
    for (j = 1; j <= num; j++) {

        switch (fork()) {
        case -1:
            exit(EXIT_FAILURE);
        case 0: /* Child */
            if (close(pfd[0]) == -1)        /* Read end is unused */
                exit(EXIT_FAILURE);
            /* Child does some work, and lets parent know it's done */
            srand(time(NULL) - j*2);
            //sleep(3);
            int t;
            do{
                t = (rand() % (MAX_VALUE+1-MIN_VALUE)) + MIN_VALUE;
            }while(!isPrime(t));
            
            printf("Child %d (PID=%ld) generated the prime number: %d\n", j, (long) getpid(), t);                             
            printf("Child %d (PID=%ld) closing pipe\n",
                     j, (long) getpid());

            if (close(pfd[1]) == -1)
                exit(EXIT_FAILURE);
            /* Child now carries on to do other things... */
            _exit(EXIT_SUCCESS);
        default: /* Parent loops to create next child */
            break;
        }
    }
    /* Parent comes here; close write end of pipe so we can see EOF */

    if (close(pfd[1]) == -1)                /* Write end is unused */
        exit(EXIT_FAILURE);
    /* Parent may do other work, then synchronizes with children */

    if (read(pfd[0], &dummy, 1) != 0)
        exit(EXIT_FAILURE);
    printf("Parent ready to go\n");
    /* Parent can now carry on to do other things... */
    exit(EXIT_SUCCESS);
}