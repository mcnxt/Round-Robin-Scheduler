/*
Author: Corey Shafer and Marcus Cureton
Assignment Number 2
Date of Submission:08/16/2022
Name of this File:rrScheduler.c
Short Description of content:
Program to schedule processes to run, using round robin scheduling algorithm in which each process gets equal
amount of time on the CPU
*/

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

struct job
{
    int proc;
    int start;
    int secondsRemaining;
    int pid;
};

void contextSwitch(struct job *nextJob);
int countLines(FILE *file);
bool jobsLeft();
struct job *processInput(char *inputFile, size_t *readJobs);
void setupTimer();
void timerHandler();

struct job *jobs;
struct job *currentJob = NULL;
size_t numberOfJobs;
// Timer sarts at 0
int secondsPassed = -10;
int jobsIter = 0;
int termJobs[15];
int termJobIter = 0;

/*
Function Name:main
Input: Program name and input file
Output(Return Value): N/A
Brief description of the task:
    Takes in a input file and sends it to processInput to retrive the
    files content. Also starts up the timer.
*/
void main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Input filename missing\n");
        exit(-1);
    }
    else if (argc > 2)
    {
        printf("Too many command line arguments\n");
        exit(-1);
    }
    else
    {
        jobs = processInput(argv[1], &numberOfJobs);
        setupTimer();
        while (1)
        {
        }
    }
}

/*
Function Name: timerHandler
Input: N/A
Output(Return Value): N/A
Brief description of the task:
Imedietly increments the timer by 10 seconds. Checks to see if the current job is
completed and checks for any new jobs ready to start. If there are no more jobs, then
 the program exits
*/
void timerHandler()
{
    secondsPassed = secondsPassed + 10;

    printf("\nScheduler: Time Now: %d seconds\n", secondsPassed);

    if (currentJob != NULL)
    {
        currentJob->secondsRemaining = currentJob->secondsRemaining - 10;
        if (currentJob->secondsRemaining < 1)
        {
            printf("Terminating job %d (PID %d)\n", currentJob->proc,
                   currentJob->pid);
            termJobs[termJobIter] = currentJob->proc;
            termJobIter++;
            kill(currentJob->pid, SIGTERM);
            currentJob = NULL;
        }
    }

    struct job *nextCandidate = NULL;
    for (int i = 0; i < numberOfJobs; i++)
    {
        if (jobs[i].secondsRemaining > 0)
        {
            if (nextCandidate == NULL)
            {
                nextCandidate = &jobs[jobsIter];
            }
        }
    }

    while (nextCandidate == NULL && !jobsLeft())
    {
        printf("No more suitable jobs\n");
        free(jobs);
        exit(0);
    }

    bool terminated = false;
    jobsIter++;
    if (jobsIter == numberOfJobs)
    {
        jobsIter = 0;
    }
    for (int i = 0; i < numberOfJobs; i++)
    {
        if (jobsIter == termJobs[i])
        {
            jobsIter++;
            if (jobsIter == numberOfJobs)
            {
                jobsIter = 0;
            }
            i = 0;
            terminated = true;
        }
    }
    contextSwitch(nextCandidate);
}

/*
Function Name:contextSwitch
Input: A pointer to the next job
Output(Return Value): N/A
Brief description of the task: Suspends the current job, checks the pd of the next job,
and resumes the right job
*/
void contextSwitch(struct job *nextJob)
{
    if (currentJob != NULL)
    { // Suspend
        printf("Suspending process %d with remaining time of %d seconds\n",
               currentJob->proc, currentJob->secondsRemaining);
        kill(currentJob->pid, SIGTSTP);
    }

    if (nextJob->pid == 0)
    {
        int pid = fork();
        if (pid == 0)
        {
            char proc_no_str[5];
            sprintf(proc_no_str, "%d", nextJob->proc);
            execlp("./prime.o", "prime.o", proc_no_str, NULL);
        }
        else
        {
            currentJob = nextJob;
            currentJob->pid = pid;
            printf("Scheduling to process %d (PID %d) whose remaining time is "
                   "%d seconds\n",
                   nextJob->proc, nextJob->pid,
                   nextJob->secondsRemaining);
        }
    }
    else
    {
        currentJob = nextJob;
        printf("Resuming process %d (PID %d) with remaining time %d\n",
               currentJob->proc, currentJob->pid, currentJob->secondsRemaining);
        kill(currentJob->pid, SIGCONT);
    }
}

/*
Function Name:jobsLeft
Input: N/A
Output(Return Value): Returns true or false depending
    on if there are any jobs remaining
Brief description of the task:Returns true or false depending
    on if there are any jobs remaining
*/
bool jobsLeft()
{
    bool jobsRemaining = false;
    for (int i = 0; i < numberOfJobs; i++)
    {
        if (jobs[i].secondsRemaining > 0)
        {
            jobsRemaining = true;
        }
    }
    return jobsRemaining;
}

/*
Function Name:countLines
Input: The file
Output(Return Value):number of lines in the file
Brief description of the task:Counts the number of lines in the file
*/
int countLines(FILE *file)
{
    rewind(file);
    int count = 0;
    char currentChar = getc(file);

    while (currentChar != EOF)
    {
        if (currentChar == '\n')
        {
            count++;
        }
        currentChar = getc(file);
    }
    return count;
}

/*
Function Name:processInput
Input: Input file and a pointer to the jobs that are read
Output(Return Value): array of jobs
Brief description of the task: Processes the input file and organizes it into job structs
*/
struct job *processInput(char *inputFile, size_t *jobsRead)
{
    FILE *in_file = fopen(inputFile, "r");
    if (in_file == NULL)
    {
        printf("Could not find requested file\n");
        exit(-1);
    }
    struct job *jobArray = malloc(sizeof(struct job) * countLines(in_file) - 1);
    // Read from the beginning of the file after counting the lines
    rewind(in_file);

    char currentLine[82];
    int loopCount = -1;
    while (fgets(currentLine, 82, in_file) != NULL)
    {
        if (loopCount < 0)
        {
            loopCount++;
        }
        else
        {
            char *currentFeild = strtok(currentLine, "\t ");
            sscanf(currentFeild, "%d", &jobArray[loopCount].proc);

            currentFeild = strtok(NULL, "\t ");
            sscanf(currentFeild, "%d", &jobArray[loopCount].secondsRemaining);

            jobArray[loopCount].pid = 0;
            loopCount++;
        }
    }
    fclose(in_file);
    *jobsRead = loopCount;
    for (int i = 0; i < numberOfJobs; i++)
    {
        termJobs[i] = -1;
    }
    return jobArray;
}

/*
Function Name:setupTimer
Input: N/A
Output(Return Value): N/A
Brief description of the task:Thie function utilizes system calls
    to handle the signals and timer
*/
void setupTimer()
{
    struct sigaction sa;
    struct itimerval timer;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timerHandler;
    sigaction(SIGALRM, &sa, NULL);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10;
    timer.it_interval.tv_sec = 10;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);
}
