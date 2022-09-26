/*
    Corey Shafer and Marcus Cureton
    2022-08-18
    prime.c
    Largest prime number finder
*/
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void contHandler(int signum);
void loop();
void termHandler(int signum);
void tstpHandler(int signum);
int check(unsigned long long int toCheck);
long long unsigned int highPrime = 99999999;
int proc;

/*
Function Name: main
Input: command and process number
Output(Return Value): N/A
Brief description of the task: takes in arguements and starts
loop to check for prime numbers
*/
void main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Missing process number\n");
        exit(-1);
    }
    else if (argc > 2)
    {
        printf("Too many command line arguments passed in\n");
        exit(-1);
    }
    else
    {
        proc = atoi(argv[1]);
        printf("Process %d: my PID is %d: I just got started. I am starting "
               "with the number %llu to find the next prime number\n",
               proc, getpid(), highPrime);

        signal(SIGTSTP, tstpHandler);
        signal(SIGCONT, contHandler);
        signal(SIGTERM, termHandler);

        loop(proc);
    }
}

/*
Function Name:lop
Input:N/A
Output(Return Value):N/A
Brief description of the task: Checks for prime numbers
*/
void loop()
{
    long long unsigned int numberToCheck = highPrime + 1;
    while (1)
    {
        if (check(numberToCheck) == 1)
        {
            highPrime = numberToCheck;
        }
        numberToCheck++;
    }
}

/*
Function Name: check
Input: number to check
Output(Return Value): true or false on if the number is prime
Brief description of the task: Returns true or false on if the number is prime
*/
int check(unsigned long long int toCheck)
{
    unsigned long int i = 2;
    int prime = 1;
    while (prime == 1 && i < toCheck / 2)
    {
        if (toCheck % i == 0)
            prime = 0;
        i++;
    }
    return (prime);
}

/*
Function Name:tstpHandler
Input: signal number
Output(Return Value):N/A
Brief description of the task: Prints message begroe suspension
*/
void tstpHandler(int signum)
{
    signal(SIGTSTP, SIG_DFL);
    printf("Process %d: my PID is %d: I am about to be suspended... Highest "
           "prime number I found is %llu.\n",
           proc, getpid(), highPrime);
    raise(SIGTSTP);
}

/*
Function Name:contHandler
Input:signal number
Output(Return Value): N/A
Brief description of the task: Prints message before resuming job
*/
void contHandler(int signum)
{
    signal(SIGCONT, contHandler);
    signal(SIGTSTP, tstpHandler);
    printf("Process %d: my PID is %d: I am about to resume... Highest "
           "prime number I found is %llu.\n",
           proc, getpid(), highPrime);
}

/*
Function Name:termHandler
Input:signal number
Output(Return Value):N/A
Brief description of the task: Prints message before terminating a job
*/
void termHandler(int signum)
{
    printf("Process %d: my PID is %d: I completed my task and I am exiting. "
           "Highest prime number I found is %llu.\n",
           proc, getpid(),
           highPrime);
    exit(0);
}
