#include "stats_functions.h"
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <utmp.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <signal.h>



void start(float cpu_usage, char* mem_usage, int delay, int samples, int sys_flag, int user_flag, int sequential_flag, int graphics_flag, float old_usage)
{
    char ram_storage[samples][1000]; // Array of strings, each element is an entry of memory usage
    char cpu_storage[samples][1000]; // Array of stings, each element is an entry of cpu usage
    int p = 0; // p is the index of cpu_storage that we will store the current cpu entry in
    int k = 0;                // K is the index of ram_storage that we will store the current memory entry in
    char cpu_store_value[1000];
    char mem_store_value[1000];
    pid_t child;
  
   



 
    for (int i = 0; i < samples; i++)
    { 
        
        int pipe1[2];

        if(pipe(pipe1) == -1){
            perror("Error while attempting to create pipe, exiting program.\n"); // Create pipe that we will be using
        }

        
        // Print sample amount of samples, if the sequential_flag is off, use escape key to refresh the screen
        if (sequential_flag == 0)
        {
            printf("\033c");
            fflush(stdout);
        }
        if(user_flag == 0){
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
            fflush(stdout);

        }
        

        if (sequential_flag == 0 && user_flag == 0)
        {
            for (int j = 0; j < i; j++)
            { // Print all of the previously stored memory usages
                printf("%s", ram_storage[j]);
                fflush(stdout);
                k = j + 1;
            }
        }


        if(user_flag == 0){

            if((child = fork()) == 0){
                if(-1 == close(pipe1[0])){
                    perror("Error closing file, exiting program.");
                    exit(0);
                }
                
                memoryInfo(mem_usage, mem_store_value, graphics_flag, old_usage, pipe1);  // Run the memoryinfo function on the child process only
                fflush(stdout);
            }
            else if(child > 0){  // On the parent process, we print the current memory info string in mem_usage
                if(-1 == close(pipe1[1])){
                    perror("Error closing file, exiting program.");
                    exit(0);
                }
                printf("%s", mem_usage);
            }
            else{
                perror("Error when forking, exiting program.\n");
                exit(1);
            }

        }



        if (sequential_flag == 0 && user_flag == 0)
        {
            for (int j = samples - 1 - i; j > 0; j--)
            { // Print enough whitespace to hold sample samples
                printf("\n");
                fflush(stdout);
            }
        }

        if (sys_flag == 0 && user_flag == 0)
        {   
            userInfo(0, i, samples);
            if(graphics_flag == 1 && sequential_flag == 0){
                for(int j = 0; j < i; j++){
                    printf("%s", cpu_storage[j]);  // print previous entries of cpu usage
                    fflush(stdout);
                    p = j + 1;
                }
                cpu_usage = cpuInfo(cpu_usage, delay, cpu_store_value, i, i, samples, sequential_flag, graphics_flag);
                for(int j = samples - 1 - i; j > 0; j--){
                    printf("\n");  // print whitespace
                    fflush(stdout);
                }
                
            }
            else if(sequential_flag == 1 && graphics_flag==1){
                cpu_usage = cpuInfo(cpu_usage, delay, cpu_store_value, -1, i, samples, sequential_flag, graphics_flag);   // Different cases depending on if graphics flag is set

            }
            else{
                cpu_usage = cpuInfo(cpu_usage, delay, NULL, -1, i, samples, sequential_flag, graphics_flag);


            }
                sysInfo();
            
        }
        else if (sys_flag == 1 && user_flag == 0)
        { // Different cases depending on --user and --system arguments
            if(graphics_flag == 1 && sequential_flag == 0){
                for(int j = 0; j < i; j++){
                    printf("%s", cpu_storage[j]);  // print previous entries of cpu usage
                    fflush(stdout);
                    p = j + 1;
                }
                cpu_usage = cpuInfo(cpu_usage, delay, cpu_store_value, i, i, samples, sequential_flag, graphics_flag);
                for(int j = samples - 1 - i; j > 0; j--){
                    printf("\n");  // print whitespace
                    fflush(stdout);
                }
                sysInfo();
            }
            else if(sequential_flag == 1 && graphics_flag==1){
                cpu_usage = cpuInfo(cpu_usage, delay, cpu_store_value, -1, i, samples, sequential_flag, graphics_flag);     // Different cases depending on if graphics flag is set
                sysInfo(); 
            }
            else{
                cpu_usage = cpuInfo(cpu_usage, delay, NULL, -1, i, samples, sequential_flag, graphics_flag);
                sysInfo();        
            }

            
        }
        else if (sys_flag == 0 && user_flag == 1)
        {   
            userInfo(delay, i, samples);
            sysInfo();
        }
        else
        {
            perror("Invalid combination of flags! Exiting program.\n");
            exit(-1);
        }
        

            if(user_flag == 0){
            int str_len;

            if(-1 == read(pipe1[0], &str_len, sizeof(int))){    // Read the length of the string that we are interested in from the pipe
                perror("error reading from pipe");
                exit(0);
            }
            
            if(-1 == read(pipe1[0], mem_store_value, str_len)){  // Read the string from the pipe and store it in mem_store_value, this is what lets the program remember the previous entries
                perror("error reading from pipe");
                exit(0);
            }
            if(-1 == read(pipe1[0], &old_usage, sizeof(float))){ // Store the old memory usage, used for graphics calculations
                perror("error reading from pipe");
                exit(0);
            }
            if(-1 == read(pipe1[0], &str_len, sizeof(int))){  // Store the length of the string that we are going to print next in str_len
                perror("error reading from pipe");
                exit(0);
            }
            if(-1 == read(pipe1[0], mem_usage, str_len)){ // Store the actual string
                perror("error reading from pipe");
                exit(0);
            }

            

        }

        if (sequential_flag == 0 && user_flag == 0)
        {   

            strcpy(ram_storage[k], mem_store_value);  // This is where we update the arrays that remember the previous iterations values
            strcpy(cpu_storage[p], cpu_store_value);

            



        }
    }
}

void handler(int sig)  // signal handler function
{
    char answer;
    // ignore the signal so we can ask user if they want to continue
    signal(SIGINT, SIG_IGN);  

    printf("Are you certain you want to exit? ('Y'/'N'): ");
    scanf(" %c", &answer);

    if (answer == 'y' || answer == 'Y')
    {
        printf("Exiting program\n");
        exit(0);
    }
    else
    {
        printf("Resuming program\n");

        
        signal(SIGINT, handler); // Restore the signal handler for SIGINT
    }
}

int main(int argc, char **argv)
{

    signal(SIGTSTP, SIG_IGN); // Ignore the ctrl z signal
    signal(SIGINT, handler); // Custom function for the ctrl c signal
    
    int delay, samples, sys_flag, user_flag, sequential_flag, graphics_flag, pos1_flag;
    delay = 1;
    samples = 10;
    sys_flag = 0; // Set default values
    user_flag = 0;
    sequential_flag = 0;
    graphics_flag = 0;
    pos1_flag = 0;
    char *temp;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--sequential") == 0)
        {
            sequential_flag = 1;
        }
        if(isdigit(argv[i][0])){
            if(pos1_flag == 0){
                samples = (int)strtol(argv[i], NULL, 10);
                pos1_flag = 1;
            }
            else if(pos1_flag == 1){
                delay = (int)strtol(argv[i], NULL, 10);
                pos1_flag = -1;
            }
        }
        if(strcmp(argv[i], "--graphics") == 0)
        {
            graphics_flag = 1;
        }
        if (strcmp(argv[i], "--user") == 0)
        {
            user_flag = 1;
        }
        if (strcmp(argv[i], "--system") == 0)
        { // iterate through the command line arguments and update flags accordingly
            sys_flag = 1;
        }
        if (strstr(argv[i], "--tdelay=") != NULL)
        {
            temp = &argv[i][0] + 9;
            delay = (int)strtol(temp, NULL, 10); // strstr(str a, str b) returns something other than null iff b is a substring of a
        }
        if (strstr(argv[i], "--samples=") != NULL)
        {
            temp = &argv[i][0] + 10;
            samples = (int)strtol(temp, NULL, 10);
        }
    }


    
    float cpu_usage = quick_cpu(delay);
    char mem_usage[1000];
    float old_usage;
    old_usage=quick_mem(mem_usage, graphics_flag);
    start(cpu_usage, mem_usage, delay, samples, sys_flag, user_flag, sequential_flag, graphics_flag, old_usage);

    return 0;
}