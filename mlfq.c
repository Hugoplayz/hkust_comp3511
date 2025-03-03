/*
    COMP3511 Fall 2024
    PA2: Multi-level Feedback Queue 
    
    Your name:CHAN Chun Hugo
    Your ITSC email:chchaneo@connect.ust.hk

    Declaration:

    I declare that I am not involved in plagiarism
    I understand that both parties (i.e., students providing the codes and students copying the codes) will receive 0 marks.

*/

// Note: Necessary header files are included
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Define MAX_NUM_PROCESS
// For simplicity, assume that we have at most 10 processes
#define MAX_NUM_PROCESS 10

#define MAX_PROCESS_NAME 5
#define MAX_GANTT_CHART 300

// N-level Feedback Queue (N=1,2,3,4)
#define MAX_NUM_QUEUE 4

// Keywords (to be used when parsing the input)
#define KEYWORD_QUEUE_NUMBER "queue_num"
#define KEYWORD_TQ "time_quantum"
#define KEYWORD_PROCESS_TABLE_SIZE "process_table_size"
#define KEYWORD_PROCESS_TABLE "process_table"

// Assume that we only need to support 2 types of space characters: 
// " " (space), "\t" (tab)
#define SPACE_CHARS " \t"

// Process data structure
// Helper functions:
//  process_init: initialize a process entry
//  process_table_print: Display the process table
struct Process {
    char name[MAX_PROCESS_NAME];
    int arrival_time ;
    int burst_time;
    int remain_time; // remain_time is needed in the intermediate steps of MLFQ 
};
void process_init(struct Process* p, char name[MAX_PROCESS_NAME], int arrival_time, int burst_time) {
    strcpy(p->name, name);
    p->arrival_time = arrival_time;
    p->burst_time = burst_time;
    p->remain_time = burst_time;
}
void process_table_print(struct Process* p, int size) {
    int i;
    printf("Process\tArrival\tBurst\n");
    for (i=0; i<size; i++) {
        printf("%s\t%d\t%d\n", p[i].name, p[i].arrival_time, p[i].burst_time);
    }
}


// A simple GanttChart structure
// Helper functions:
//   gantt_chart_print: display the current chart
struct GanttChartItem {
    char name[MAX_PROCESS_NAME];
    int duration;
};

void gantt_chart_print(struct GanttChartItem chart[MAX_GANTT_CHART], int n) {
    int t = 0;
    int i = 0;
    printf("Gantt Chart = ");
    printf("%d ", t);
    for (i=0; i<n; i++) {
        t = t + chart[i].duration;     
        if(i + 1 < n)
        {
            if(!strcmp(chart[i].name,chart[i+1].name))
            { 
                continue;
            }
        }
        if(!chart[i].duration) continue;
        printf("%s %d ", chart[i].name, t);
    }
    printf("\n");
}

// Global variables
int queue_num = 0;
int process_table_size = 0;
struct Process process_table[MAX_NUM_PROCESS];
int time_quantum[MAX_NUM_QUEUE];


// Helper function: Check whether the line is a blank line (for input parsing)
int is_blank(char *line) {
    char *ch = line;
    while ( *ch != '\0' ) {
        if ( !isspace(*ch) )
            return 0;
        ch++;
    }
    return 1;
}
// Helper function: Check whether the input line should be skipped
int is_skip(char *line) {
    if ( is_blank(line) )
        return 1;
    char *ch = line ;
    while ( *ch != '\0' ) {
        if ( !isspace(*ch) && *ch == '#')
            return 1;
        ch++;
    }
    return 0;
}
// Helper: parse_tokens function
void parse_tokens(char **argv, char *line, int *numTokens, char *delimiter) {
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}

// Helper: parse the input file
void parse_input() {
    FILE *fp = stdin;
    char *line = NULL;
    ssize_t nread;
    size_t len = 0;

    char *two_tokens[2]; // buffer for 2 tokens
    char *queue_tokens[MAX_NUM_QUEUE]; // buffer for MAX_NUM_QUEUE tokens
    int n;

    int numTokens = 0, i=0;
    char equal_plus_spaces_delimiters[5] = "";

    char process_name[MAX_PROCESS_NAME];
    int process_arrival_time = 0;
    int process_burst_time = 0;

    strcpy(equal_plus_spaces_delimiters, "=");
    strcat(equal_plus_spaces_delimiters,SPACE_CHARS);    

    // Note: MingGW don't have getline, so you are forced to do the coding in Linux/POSIX supported OS
    // In other words, you cannot easily coding in Windows environment

    while ( (nread = getline(&line, &len, fp)) != -1 ) {
        if ( is_skip(line) == 0)  {
            line = strtok(line,"\n");

            if (strstr(line, KEYWORD_QUEUE_NUMBER)) {
                // parse queue_num
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    sscanf(two_tokens[1], "%d", &queue_num);
                }
            } 
            else if (strstr(line, KEYWORD_TQ)) {
                // parse time_quantum
                parse_tokens(two_tokens, line, &numTokens, "=");
                if (numTokens == 2) {
                    // parse the second part using SPACE_CHARS
                    parse_tokens(queue_tokens, two_tokens[1], &n, SPACE_CHARS);
                    for (i = 0; i < n; i++)
                    {
                        sscanf(queue_tokens[i], "%d", &time_quantum[i]);
                    }
                }
            }
            else if (strstr(line, KEYWORD_PROCESS_TABLE_SIZE)) {
                // parse process_table_size
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    sscanf(two_tokens[1], "%d", &process_table_size);
                }
            } 
            else if (strstr(line, KEYWORD_PROCESS_TABLE)) {

                // parse process_table
                for (i=0; i<process_table_size; i++) {

                    getline(&line, &len, fp);
                    line = strtok(line,"\n");  

                    sscanf(line, "%s %d %d", process_name, &process_arrival_time, &process_burst_time);
                    process_init(&process_table[i], process_name, process_arrival_time, process_burst_time);

                }
            }

        }
        
    }
}
// Helper: Display the parsed values
void print_parsed_values() {
    printf("%s = %d\n", KEYWORD_QUEUE_NUMBER, queue_num);
    printf("%s = ", KEYWORD_TQ);
    for (int i=0; i<queue_num; i++)
        printf("%d ", time_quantum[i]);
    printf("\n");
    printf("%s = \n", KEYWORD_PROCESS_TABLE);
    process_table_print(process_table, process_table_size);
}

// TODO: Implementation of MLFQ algorithm
void mlfq() {

    struct GanttChartItem chart[MAX_GANTT_CHART];
    int sz_chart = 0; 

    // TODO: Write your code here to implement MLFQ
    // Tips: A simple array is good enough to implement a queue
    struct Process queue[4][10];
    struct Process buff;
    int total_time = 0;
    int current_time = 0;
    int nums[4] = {0,0,0,0};
    int duration[4] = {0,0,0,0};
    int solo_timecount = 0;
    int last_queue = 0;
    for (int i = 0; i < process_table_size; i++)
    {
        total_time += process_table[i].burst_time;
    }
    while (current_time != total_time + 1)
    {
        for (int i = 0; i < process_table_size; i++)
        {
            if (process_table[i].arrival_time == current_time)//add to queue
            {
                queue[0][nums[0]] = process_table[i];
                nums[0]++;
            }
            
        }
        for (int k = 0; k < queue_num; k++)//left queue or change queue
        {
            if (nums[k]) //queue is not empty
            {
                if (queue[k][0].remain_time == 0)
                {

                    chart[sz_chart].duration = solo_timecount;
                    strcpy(chart[sz_chart].name,queue[k][0].name);
                    sz_chart++;
                    for (int i = 0; i < nums[k] - 1; i++)
                    {
                        queue[k][i] = queue[k][i+1];
                    }
                    nums[k]--;
                    duration[k] = 0;
                    solo_timecount = 0;
                    
                }
                else if (duration[k] == time_quantum[k])
                {
                    chart[sz_chart].duration = solo_timecount;
                    strcpy(chart[sz_chart].name,queue[k][0].name);
                    sz_chart++;
                    buff = queue[k][0];
                    for (int i = 0; i < nums[k] - 1; i++)
                    {
                        queue[k][i] = queue[k][i+1];
                    }
                    queue[k+1][nums[k+1]] = buff;
                    nums[k+1]++;
                    nums[k]--;
                    duration[k] = 0;
                    solo_timecount = 0;
                }
            }
        }
        for(int k = 0; k < queue_num; k++)//process
        {
                if (nums[k])
                {
                    if (last_queue > k)
                    {
                        if(solo_timecount != 0)
                        {
                            chart[sz_chart].duration = solo_timecount;
                            strcpy(chart[sz_chart].name,queue[last_queue][0].name);
                            solo_timecount = 0;
                            sz_chart++;
                        }

                    }
                    
                    queue[k][0].remain_time--;
                    duration[k]++;
                    solo_timecount++;
                    last_queue = k;
                    break;
                }
                
                

        }   
            
        
        current_time++;
    }
    
    
  
    
    // At the end, display the final Gantt chart
    gantt_chart_print(chart, sz_chart);

}


int main() {
    parse_input();
    print_parsed_values();
    mlfq();
    return 0;
}