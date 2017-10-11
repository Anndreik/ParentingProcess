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
    
    int j;
   
    char buf[BUF_SIZE];                     //buffer to use for reading;
    char buf_write[BUF_SIZE];               //buffer to use for writing;   
    ssize_t numRead;                        //how much to read
    int status = 0;
    pid_t wpid;

    if (argc < 2 || strcmp(argv[1], "--help") == 0){
        printf("Usage: %s <number_of_childs>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    int num;
    num = strtol(argv[1], NULL, 10);
    int pfd[num][2];                             /* Process synchronization pipe */
    setbuf(stdout, NULL);                   /* Make stdout unbuffered, since we
                                               terminate child with _exit() */
    printf("Parent started\n");
    int x;
    for(x = 0; x < num; ++x){
        if (pipe(pfd[x]) == -1){
            printf("Creating pipe %d failed!\n", x);
            exit(EXIT_FAILURE);
        }
    }
    
    
    for (j = 1; j <= num; j++) {

        switch (fork()) {
        case -1:
            printf("error no= %d, ERROR = %s \n",errno,strerror(errno));
            exit(EXIT_FAILURE);
        case 0: /* Child */
            if (close(pfd[j-1][0]) == -1){        /* Read end is unused */
                printf("Child closing read end of pipe %d failed!\n", j-1);
                exit(EXIT_FAILURE);
            }
            
            srand(time(NULL) - j*2);
            
            int t;
            do{
                t = (rand() % (MAX_VALUE+1-MIN_VALUE)) + MIN_VALUE;
            }while(!isPrime(t));
             
            //now write the value on the pipe
            snprintf(buf_write, BUF_SIZE, "Child process %ld send the prime number: %d\n", (long) getpid(), t);
            //printf ("This is buf_write: %s\n", buf_write);
            
            //if (write(pfd[j-1][1], buf_write, (strlen(buf_write)+1))){
            //}
            int bsent = 0;
            int temp = 0;
			while(bsent < strlen(buf_write))
			{
                temp = write(pfd[j-1][1], buf_write+bsent, 5);
                if(temp == -1){
                    printf("Child write on pipe %d failed", j-1);
                    exit(EXIT_FAILURE);
                }
                else if(temp == 0){
                    break;
                }
                else if(temp > 0 && temp <= 5)
				    bsent += temp;
				sleep(1);
			}


            if (close(pfd[j-1][1]) == -1){
                printf("Child closing write end of pipe %d failed!\n", j-1);
                exit(EXIT_FAILURE);
            }
                

            _exit(EXIT_SUCCESS);
        default: /* Parent loops to create next child */
            break;
        }
    }
    /* Parent comes here; close write end of pipe so we can see EOF */
    int y;
    for(y = 0; y < num; ++y){
        if (close(pfd[y][1]) == -1){               /* Write end is unused */
            printf("Parent closing write end of pipe %d failed!\n", y);
            exit(EXIT_FAILURE);
        }
    }
                    
        
    

                  /* Read data from pipe, echo on stdout */
        int z;
        int w;
        for(z = 0; z < num; ++z){
            for (;;) {
                numRead = read(pfd[z][0], buf, BUF_SIZE);
                if (numRead == -1)
                    exit(EXIT_FAILURE);
                if (numRead == 0)
                    break;                      /* End-of-file */
                w = write(STDOUT_FILENO, buf, numRead);
                if (w == -1){
                    printf("Error writing to STDOUT_FILENO");
                    exit(EXIT_FAILURE);
                }
                else if(w == 0){
                    printf("Parent has nothing to write on pipe %d\n", z);
                }
                
            }
        }
    
    while ((wpid = wait(&status)) > 0);    
    
    exit(EXIT_SUCCESS);
}
