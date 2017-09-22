Ex 2 - parenting process

Write a program that creates N number of child processes. The N is given as a command line argument and should be between 1 and 10.

Each child process will generate random numbers in a loop until the number generated is prime. When this happens it sends back to the parent process a message containing its PID and the prime number. Then it exits.
The parent process creates all the child processes and then waits for the messages. Each time it receives a message, it prints out it content. After it received the expected number of messages it proceeds to wait on all the child processes in order to not create orphans and then it terminates.

To send the messages back to the parent process, use a pipe. The child processes will inherit the file descriptors and they will be able to send data back. You can find details of this technique in The Linux Programming Interface.
