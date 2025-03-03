/*
    COMP3511 Fall 2024
    PA1: Simplified Linux Shell (MyShell)

    Your name:
    Your ITSC email:           ihchow@connect.ust.hk

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
void read_tokens(char **argv, char *line, int *numTokens, char *delimiter)          // prompt  with "|" pipe symbol
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


// Define the process_cmd function from given

void process_cmd(char *command_line)
{
    char *pipe_segments[MAX_PIPE_SEGMENTS];
    int num_pipe_segments;

    char *arguments[MAX_ARGUMENTS_PER_SEGMENT] = {NULL};
    int num_arguments;
    int pfds[2];            //pipe siz, 1 is write, 0 is read

    // Split the command by the pipe character
    read_tokens(pipe_segments, command_line, &num_pipe_segments, "|");            //PIPE_CHAR

    for (int i = 0; i < num_pipe_segments; i++) {           //scan throuhgh the pipe segments
        read_tokens(arguments, pipe_segments[i], &num_arguments, SPACE_CHARS);              // collect the arguments by space
        arguments[num_arguments] = NULL;            //set the last argument to NULL( C requirement)

        // If the command contains a redirection symbol (> or <)                    feature 3 file I/O redirection
        for (int j = 0; j < num_arguments; j++) {
            if (strcmp(arguments[j], "<") == 0) {
                int file = open(arguments[j+1], O_RDONLY, S_IRUSR | S_IWUSR);         //open the file
                dup2(file, STDIN_FILENO);
                close(file);

                arguments[j] = NULL;
            } else if (strcmp(arguments[j], ">") == 0) {
                int file2 = open(arguments[j+1], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);        //open the file
                dup2(file2, STDOUT_FILENO);
                close(file2);

                arguments[j] = NULL;
            }
        }

        // Check if the command is not the last command in the entire command line input
        if (i+1 != num_pipe_segments) {
            pipe(pfds);                                     //create a pipe
            pid_t pid = fork();                              

            if (pid == 0) {                          //child
                close(STDOUT_FILENO);
                dup2(pfds[1], STDOUT_FILENO);
                close(pfds[0]);

                execvp(arguments[0], arguments);
            } else {                                //parent
                close(STDIN_FILENO);
                dup2(pfds[0], 0);
                close(pfds[1]);

                wait(0);
            }
        } else {
            execvp(arguments[0], arguments);
        }
    }
}




void signal_callback (int sig){
    printf(TEMPLATE_MYSHELL_TERMINATE, getpid());
    exit(1);
    
}



/* The main function implementation */
int main()
{
    // TODO: replace the shell prompt with your ITSC account name
    // For example, if you ITSC account is cspeter@connect.ust.hk
    // You should replace ITSC with cspeter
    char *prompt = "ihchow";
    char command_line[MAX_CMDLINE_LENGTH];

    fgets(command_line, MAX_CMDLINE_LENGTH, stdin);
    process_cmd(command_line);
 
    // TODO:
    // The main function needs to be modified
    // For example, you need to handle the exit command inside the main function

    printf(TEMPLATE_MYSHELL_START, getpid());

    // The main event loop
    while (1)
    {
    
        printf("%s> ", prompt);
        if (get_cmd_line(command_line) == -1)
            continue; /* empty line handling */
        

        //ctrl-c exit using signal_callback
        signal(SIGINT, signal_callback);


        if (strcmp(command_line, "exit") == 0)          // feature 1: exit
        {
            printf(TEMPLATE_MYSHELL_END, getpid());
            break;
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            // the child process handles the command
            process_cmd(command_line);
        }
        else
        {
            // the parent process simply wait for the child and do nothing
            wait(0);
        }
    }

    return 0;
}

