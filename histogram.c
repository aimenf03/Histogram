#define _POSIX_C_SOURCE 200809L
#define NUM_CHILDREN 10
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

int pipes[NUM_CHILDREN][2];
pid_t child_pids[NUM_CHILDREN];
int num_children = 0;

// SIGCHLD signal handler
void sigchld_handler(int signum __attribute__((unused))) //allows unused functions to run without warnings
{
    pid_t pid;
    int child_status;

    while ((pid = waitpid(-1, &child_status, WNOHANG)) > 0) 
    {
        printf("PID: %d \n", pid);
        if (WIFEXITED(child_status) && WEXITSTATUS(child_status) == 0) //successfully terminates child 
        {
            int i;
            for (i = 0; i < num_children; i++) 
            {
                if (child_pids[i] == pid) 
                {
                    //reads histogram from pipes
                    int histogram[26];
                    close(pipes[i][1]);
                    read(pipes[i][0], histogram, sizeof(histogram));
                    close(pipes[i][0]);

                    //function writes histogram to output file
                    char filename[20];
                    sprintf(filename, "file%d.hist", pid);
                    FILE *output_file = fopen(filename, "w");
                    if (output_file != NULL) {
                        int j;
                        for (j = 0; j < 26; j++) {
                            fprintf(output_file, "%c %d\n", 'a' + j, histogram[j]);
                        }
                        fclose(output_file);
                        printf("The histogram generated to file: %s\n", filename);
                    } else {
                        printf("Failed to open output file %s\n", filename);
                    }
                    break;
                }
            }
        }
        //terminated child PID is removed
        int i, j;
        for (i = 0; i < num_children; i++) 
        {
            if (child_pids[i] == pid) 
            {
                for (j = i; j < num_children - 1; j++) 
                {
                    child_pids[j] = child_pids[j + 1];
                    pipes[j][0] = pipes[j + 1][0];
                    pipes[j][1] = pipes[j + 1][1];
                }
                num_children--;
                break;
            }
        }
    }
}

//characters in a file are computed into histograms
void child_process(char *filename, int index) 
{
    sleep(10 + 3 * index);
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        int histogram[26] = {0};
        int c;
        while ((c = fgetc(file)) != EOF) 
        {
            if (isalpha(c)) //checks for alphabetical order for characters
            {
                c = tolower(c); //coverts to lower case for alphabets
                histogram[c - 'a']++;
            }
        }
        fclose(file); 
        close(pipes[index][0]);
        write(pipes[index][1], histogram, sizeof(histogram));
        close(pipes[index][1]);
        exit(0);
    } else 
    {
        close(pipes[index][0]);
        exit(1);
    }
}

int main(int argc, char *argv[]) 
{
    signal(SIGCHLD, sigchld_handler); //signal for SIGCHLD handler

    if (argc < 2) //checks if arguments are given
    {
        printf("Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) //fork child processes
    {
        if (num_children >= NUM_CHILDREN) 
        {
            printf("Maximum number of children reached\n");
            break;
        }

        //pipe for communication with child is created
        if (pipe(pipes[num_children]) == -1) 
        {
            printf("Failed to create pipe\n");
            break;
        }

        pid_t pid = fork(); //fork child process
        if (pid == 0) 
        {
            child_process(argv[i], num_children);
        } else if (pid > 0) 
        {
            printf("The spawned child process for file: %s\n", argv[i]);
            child_pids[num_children++] = pid;
        } else 
        {
            printf("Failed to fork child process\n");
        }
    }

    while (num_children > 0) //waits for all children processes to terminate
    { 
        sleep(1);
    }

    return 0;
}