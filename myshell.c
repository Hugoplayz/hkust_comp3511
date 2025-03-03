/*
    COMP3511 Fall 2024
    PA1: Simplified Linux Shell (MyShell)

    Your name:CHAN, Chun Hugo
    Your ITSC email:chchaneo@connect.ust.hk

    Declaration:

    I declare that I am not involved in plagiarism
    I understand that both parties (i.e., students providing the codes and students copying the codes) will receive 0 marks.

*/

/*
    Header files for MyShell
    Necessary header files are included.
    Do not include extra header files
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> // For constants that are required in open/read/write/close syscalls
#include <sys/wait.h> // For wait() - suppress warning messages
#include <fcntl.h>    // For open/read/write/close syscalls
#include <signal.h>   // For signal handling

// Define template strings so that they can be easily used in printf
//
// Usage: assume pid is the process ID
//
//  printf(TEMPLATE_MYSHELL_START, pid);
//
#define TEMPLATE_MYSHELL_START "Myshell (pid=%d) starts\n"
#define TEMPLATE_MYSHELL_END "Myshell (pid=%d) ends\n"
#define TEMPLATE_MYSHELL_TERMINATE "Myshell (pid=%d) terminates by Ctrl-C\n"

// Assume that each command line has at most 256 characters (including NULL)
#define MAX_CMDLINE_LENGTH 256

// Assume that we have at most 8 arguments
#define MAX_ARGUMENTS 8

// Assume that we only need to support 2 types of space characters:
// " " (space) and "\t" (tab)
#define SPACE_CHARS " \t"

// The pipe character
#define PIPE_CHAR "|"

// Assume that we only have at most 8 pipe segements,
// and each segment has at most 256 characters
#define MAX_PIPE_SEGMENTS 8

// Assume that we have at most 8 arguments for each segment
// We also need to add an extra NULL item to be used in execvp
// Thus: 8 + 1 = 9
//
// Example:
//   echo a1 a2 a3 a4 a5 a6 a7
//
// execvp system call needs to store an extra NULL to represent the end of the parameter list
//
//   char *arguments[MAX_ARGUMENTS_PER_SEGMENT];
//
//   strings stored in the array: echo a1 a2 a3 a4 a5 a6 a7 NULL
//
#define MAX_ARGUMENTS_PER_SEGMENT 9

// Define the standard file descriptor IDs here
#define STDIN_FILENO 0  // Standard input
#define STDOUT_FILENO 1 // Standard output

// This function will be invoked by main()
// This function is given
int get_cmd_line(char *command_line)
{
    int i, n;
    if (!fgets(command_line, MAX_CMDLINE_LENGTH, stdin))
        return -1;
    // Ignore the newline character
    n = strlen(command_line);
    command_line[--n] = '\0';
    i = 0;
    while (i < n && command_line[i] == ' ')
    {
        ++i;
    }
    if (i == n)
    {
        // Empty command
        return -1;
    }
    return 0;
}

// read_tokens function is given
// This function helps you parse the command line
//
// Suppose the following variables are defined:
//
// char *pipe_segments[MAX_PIPE_SEGMENTS]; // character array buffer to store the pipe segements
// int num_pipe_segments; // an output integer to store the number of pipe segment parsed by this function
// char command_line[MAX_CMDLINE_LENGTH]; // The input command line
//
// Sample usage:
//
//  read_tokens(pipe_segments, command_line, &num_pipe_segments, "|");
//
void read_tokens(char **argv, char *line, int *numTokens, char *delimiter)
{
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}

void process_cmd(char *command_line)
{
    // Uncomment this line to check the cmdline content
    // Please remember to remove this line before the submission
    printf("Debug: The command line is [%s]\n", command_line);
}

void  INThandler(int sig)
{


    signal(sig, SIG_IGN);
    printf(TEMPLATE_MYSHELL_TERMINATE,getpid());
    exit(1);
    
}


/* The main function implementation */
int main()
{
    // TODO: replace the shell prompt with your ITSC account name
    // For example, if you ITSC account is cspeter@connect.ust.hk
    // You should replace ITSC with cspeter
    char *prompt = "chchaneo";
    char command_line[MAX_CMDLINE_LENGTH];
    char **processed_command_line = malloc(MAX_ARGUMENTS_PER_SEGMENT * sizeof(char *));
    char **processed_pipeline_cmd = malloc(MAX_PIPE_SEGMENTS * sizeof(char *));
    int pipenums,nums;
    int pfds[2];
    int running = 1;
    int in;


    // TODO:
    // The main function needs to be modified
    // For example, you need to handle the exit command inside the main function

    printf(TEMPLATE_MYSHELL_START, getpid());
    signal(SIGINT, INThandler);

    // The main event loop
    while (running)
    {

        printf("%s> ", prompt);
        if (get_cmd_line(command_line) == -1)
            continue; /* empty line handling */

        read_tokens(processed_pipeline_cmd,command_line,&pipenums,PIPE_CHAR);
        for (int i = 0; i < pipenums; i++)
        {
            if (!running)
            {
                break;
            }
            
            read_tokens(processed_command_line,processed_pipeline_cmd[i],&nums,SPACE_CHARS);
            processed_command_line[nums] = NULL;
            if (i < pipenums - 1) pipe(pfds);
            int status;
            pid_t pid = fork();
            if (pid == 0)
            {
                for (int j = 0; j < nums; j++) {
                    if (strcmp(processed_command_line[j], "<") == 0) {
                        int fd = open(processed_command_line[j+1], O_RDONLY, S_IRUSR | S_IWUSR);
                        dup2(fd, STDIN_FILENO);
                        close(fd);

                        processed_command_line[j] = NULL;
                    } else if (strcmp(processed_command_line[j], ">") == 0) {
                        int fd = open(processed_command_line[j+1], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
                        dup2(fd, STDOUT_FILENO);
                        close(fd);

                        processed_command_line[j] = NULL;
                    }
                }
                if (i)//not first get input from pipe
                {
                    dup2(in , STDIN_FILENO);
                    close(pfds[0]);
                }
                if (i < pipenums - 1)// not last write output to pipe
                {
                    dup2(pfds[1],STDOUT_FILENO);
                    close(pfds[1]);
                }

                if (strcmp(processed_command_line[0], "exit") == 0) {
                        
                        exit(1);
                }

                execvp(processed_command_line[0], processed_command_line);
                exit(0); 
            }
            else{
                wait(&status);
                if (pipenums != 1)
                {
                    close(pfds[1]);
                    in = pfds[0];
                }
                

                if (WEXITSTATUS(status))
                {
                    printf(TEMPLATE_MYSHELL_END, getpid());
                    running = 0;
                }
            } 
        }
    }
    free(processed_command_line);
    free(processed_pipeline_cmd);

    return 0;
}