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



void sysInfo()
{
    printf("\n---------------------------------------\n");
    struct utsname unameinfo; // sysinfo struct, its fields will be set by passing a pointer to it in uname()
    uname(&unameinfo);
    printf("### System Information ###\n System Name = %s\n Machine Name = %s\n Version = %s\n Release = %s\n Architecture = %s\n---------------------------------------\n",
           unameinfo.sysname, unameinfo.nodename, unameinfo.version, unameinfo.release, unameinfo.machine);
}

float quick_cpu(int delay)
{ // This function is needed in order to give us our first cpu_usage value
    FILE *usage = fopen("/proc/stat", "r");
    if (usage == NULL)
    {
        perror("Unable opening file, terminating program.\n");
        exit(-1);
    }
    char temp[1000];

long long total_time, total_idle_time, total_usage_time, total_time2, total_idle_time2, total_usage_time2, user_time, nice_time, system_time, idle_time, iowait_time, irq_time, softirq_time, steal_time, guest_time, guest_nice_time;

    fgets(temp, 1000, usage); // Put the first line of the /proc/stat file into temp

    for (int i = 0; i < 1000; i++)
    {
        if (!isdigit(temp[i]) && temp[i] != '\0')
        { // Replace all non digit characters with whitespace so we can use scanf
            temp[i] = ' ';
        }
    }
    // scanf only works with stdin, so we need sscanf if we want the input to be a hardcoded string

    sscanf(temp, " %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld", &user_time, &nice_time, &system_time, &idle_time, &iowait_time, &irq_time, &softirq_time, &steal_time, &guest_time, &guest_nice_time);
    total_time = user_time + nice_time + system_time + idle_time + iowait_time + irq_time + softirq_time;
    total_idle_time = idle_time;
    total_usage_time = total_time - total_idle_time;

    if(fclose(usage) != 0){
        perror("Error closing file, terminating program.\n");
        exit(-1);
    }; // Close and reopen after delay so we get an updated version
    sleep(delay);
    FILE *usage_updated = fopen("/proc/stat", "r");

    if(usage_updated == NULL){
        perror("Error opening file, terminating program.\n");
        exit(-1);
    }

    fgets(temp, 1000, usage_updated);

    for (int i = 0; i < 1000; i++)
    {
        if (!isdigit(temp[i]) && temp[i] != '\0')
        { // Replace all non digit characters with whitespace so we can use scanf
            temp[i] = ' ';
        }
    }

    sscanf(temp, " %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld", &user_time, &nice_time, &system_time, &idle_time, &iowait_time, &irq_time, &softirq_time, &steal_time, &guest_time, &guest_nice_time);

    total_time2 = user_time + nice_time + system_time + idle_time + iowait_time + irq_time + softirq_time;
    total_idle_time2 = idle_time;
    total_usage_time2 = total_time2 - total_idle_time2;

    long long total_usage_dif = total_usage_time2 - total_usage_time;
    long long total_time_dif = total_time2 - total_time;

    float cpu_usage = (float)total_usage_dif / total_time_dif; // Same cpu usage formula as explained above
    cpu_usage *= 100;

    if(fclose(usage_updated) != 0){
        perror("Error closing file, terminating program.\n");
        exit(-1);
    }
    

    return cpu_usage;
}

float quick_mem(char *placeholder, int graphics){  // This function is used to get the first memory info string

    char temp[1000];
    struct sysinfo ram_info;
    sysinfo(&ram_info);
    
    float total_physical = (9.31 * pow(10, -10)) * (ram_info.totalram / ram_info.mem_unit); // the 9.31 * pow(10,-10) converts it to GB
    float current_usage = (9.31 * pow(10, -10)) * (ram_info.totalram - ram_info.freeram);
    float total_virtual = (9.31 * pow(10, -10)) * ((ram_info.totalram + ram_info.totalswap) / ram_info.mem_unit);
    if(graphics == 1){
        sprintf(temp, "\n%.2fGB / %.2fGB  --  %.2fGB / %.2fGB\t|o 0.00 (%.2f)", current_usage, total_physical, current_usage, total_virtual, current_usage);

    }
    else{
        sprintf(temp, "\n%.2fGB / %.2fGB  --  %.2fGB / %.2fGB", current_usage, total_physical, current_usage, total_virtual);
    }
    

    strcpy(placeholder, temp);  // placeholder is a string that will be stored in our cpu_storage array so that we remember the previous iterations
    return current_usage;



}

void memoryInfo(char *print_this, char *placeholder, int graphics_flag, float old_usage, int* mem_pipe)
{

    char storage[1000];

    int str_len = strlen(print_this) + 1;

    if(-1 == write(mem_pipe[1], &str_len, sizeof(str_len))){       // Write the length of the string that is to be printed to the pipe
        perror("Error writing to pipe, exiting program.\n");
        kill(getppid(),SIGKILL);
        exit(1);
    }

    if(-1 == write(mem_pipe[1], print_this, str_len)){
        perror("Error writing to pipe, exiting program.\n");   // Write the actual string
        kill(getppid(),SIGKILL);
        exit(1);
    }
    
    
    fflush(stdout);
    
    float thing = (int)(old_usage * 100 + .5);           // This is an algorithm used to round a float to 2 decimal places
    old_usage = (float)thing / 100;
    

    
    struct sysinfo ram_info;
    sysinfo(&ram_info);
    float total_physical = (9.31 * pow(10, -10)) * (ram_info.totalram / ram_info.mem_unit); // the 9.31 * pow(10,-10) converts it to GB
    float current_usage = (9.31 * pow(10, -10)) * (ram_info.totalram - ram_info.freeram);
    float total_virtual = (9.31 * pow(10, -10)) * ((ram_info.totalram + ram_info.totalswap) / ram_info.mem_unit);
    thing = (int)(current_usage * 100 + .5);
    current_usage = (float)thing / 100;
    if(graphics_flag == 1){

        float difference = current_usage - old_usage;
        thing = (int)(difference * 100 + .5);
        difference = (float)thing / 100;

        if(difference > 0){
            int num_symbols = (int)(difference*100);

            sprintf(storage, "\n%.2fGB / %.2fGB  --  %.2fGB / %.2fGB\t|%.*s* %.2f (%.2f)", current_usage, total_physical, current_usage,
            total_virtual, num_symbols,
            "#######################################################################################"
            "#######################################################################################"        // the number of symbols that are printed depends on the difference in memory usage
            "#######################################################################################"
            "#######################################################################################"
            "#######################################################################################"
            "#######################################################################################"
            "#######################################################################################", current_usage - old_usage, current_usage);
        }
        else if (current_usage < old_usage){
            
            sprintf(storage, "\n%.2fGB / %.2fGB  --  %.2fGB / %.2fGB\t|%.*s@ %.2f (%.2f)", current_usage, total_physical, current_usage,
            total_virtual, (int)(-100*(current_usage - old_usage)),
            ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
            ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
            ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"   // the number of symbols that are printed depends on the difference in memory usage
            ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
            ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
            ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
            ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::", current_usage - old_usage, current_usage);
        }
        else{
            sprintf(storage, "\n%.2fGB / %.2fGB  --  %.2fGB / %.2fGB\t|o 0.00 (%.2f)", current_usage, total_physical, current_usage, total_virtual, current_usage);
        }
    }else{

    
        sprintf(storage, "\n%.2fGB / %.2fGB  --  %.2fGB / %.2fGB", current_usage, total_physical, current_usage, total_virtual);

    }


    if(-1 == write(mem_pipe[1], &current_usage, sizeof(current_usage))){   // Write the current cpu usage to the pipe, so that we can make use of it in the next iteartion for graphics
    perror("Error writing to pipe, exiting program.\n");
    kill(getppid(),SIGKILL);
    exit(1);
    }
    str_len = strlen(storage) + 1;
    if(-1 == write(mem_pipe[1], &str_len, sizeof(str_len))){
        perror("Error writing to pipe, exiting program.\n"); // Write the length of the cpu usage summary string we just calculated to the pipe
        kill(getppid(),SIGKILL);
        exit(1);
    }
    if(-1 == write(mem_pipe[1], storage, str_len)){
        perror("Error writing to pipe, exiting program.\n"); // Write the actual string to the pipe
        kill(getppid(),SIGKILL);
        exit(1);
    }
    if(-1 == close(mem_pipe[1])){
        perror("Error closing pipe, exiting program.\n");
        exit(1);
    };  // Close the write end of the pipe and exit, becasue we are done with this child process
    exit(0);
  
}


float cpuInfo(float previous, int delay, char* placeholder, int flag, int iteration, int samples, int sequential_flag, int graphics_flag)
{

    FILE *coreinfo = fopen("/proc/cpuinfo", "r"); // File for calculating # of cores
    if (coreinfo == NULL)
    {
        perror("Unable to open file, terminating program.\n");
        exit(-1);
    }

    char temp[1000]; // String that will store each line of cpuinfo
    char storage[1000];



    while (fgets(temp, 1000, coreinfo) != NULL)
    { // This while loop will break when we reach the line directly above the line that we are intersted in

        if (strstr(temp, "core id") != NULL)
        { // if "core id" appears in temp, break
            break;
        }
    }

    fgets(temp, 1000, coreinfo); // Put the value of the line of interest into temp

    int j;
    for (int i = 0; temp[i] != ':'; i++)
    { // Set j such that the address of ':' in temp is equal to &temp[0] + j + 1
        j = i;
    }
    j = j + 2;                    // Increment by 2 bytes so that &temp[0] + j is the address of the whitespace character after the ':' and before the first digit

    char *core_arr = temp + j; // Create new string starting at the whitespace before the first digit as explained in the line above so that we can use strtol

    long int cores = strtol(core_arr, NULL, 10);
    
    if(placeholder == NULL || flag == 0 || sequential_flag == 1){
        printf("\nNumber of cores: %ld\n", cores);

    }


    if(sequential_flag == 1 && placeholder != NULL){

        printf("CPU usage:\n");

    } 
    if(placeholder == NULL){

        printf("CPU usage:\n%f%%\n", previous);

    }  // Print the previously generated cpu usage %
    else if(placeholder != NULL && flag == 0){
        int num_bars = (int)previous;
        if(num_bars < 1) num_bars = 1;


         printf("CPU usage:\n\n%f%%\t%.*s\n", previous, num_bars,
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");
        sprintf(storage, "\nNumber of cores: %ld\nCPU usage:\n\n%f%%\t%.*s\n", cores, previous, num_bars,  // "%.*s" lets us set the amount of bars printed to be equal to num_bars
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");

    strcpy(placeholder, storage);       // Placeholder is the string that will be stored in the cpu_storage array so that we can remember them for next iterations

    } if(placeholder != NULL && flag != 0){

        printf("%f%%", previous);
        int num_bars = (int)previous;
        if(num_bars < 1) num_bars = 1;

         printf("\t%.*s\n", num_bars,
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||" // "%.*s" lets us set the amount of bars printed to be equal to num_bars
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");
        sprintf(storage, "%f%%\t%.*s\n", previous, num_bars,
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
         "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");


    strcpy(placeholder, storage);  // Placeholder is the string that will be stored in the cpu_storage array so that we can remember them for next iterations
    }
    




    if(iteration+1 == samples) return 0; // There is no need to calculate another value if we have already printed enough samples, so return
    


    FILE *usage = fopen("/proc/stat", "r");
    if (usage == NULL)
    {
        perror("Unable to open file, terminating program.\n");
        exit(-1);
    }

    long long total_time, total_idle_time, total_usage_time, total_time2, total_idle_time2, total_usage_time2, user_time, nice_time, system_time, idle_time, iowait_time, irq_time, softirq_time, steal_time, guest_time, guest_nice_time;

    fgets(temp, 1000, usage); // Put the first line of the /proc/stat file into temp

    for (int i = 0; i < 1000; i++)
    {
        if (!isdigit(temp[i]) && temp[i] != '\0')
        { // Replace all non digit characters with whitespace so we can use scanf
            temp[i] = ' ';
        }
    }
    // scanf only works with stdin, so we need sscanf if we want the input to be a hardcoded string

    sscanf(temp, " %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld", &user_time, &nice_time, &system_time, &idle_time, &iowait_time, &irq_time, &softirq_time, &steal_time, &guest_time, &guest_nice_time);
    total_time = user_time + nice_time + system_time + idle_time + iowait_time + irq_time + softirq_time;
    total_idle_time = idle_time;
    total_usage_time = total_time - total_idle_time;

    if(fclose(usage) != 0){
        perror("Error closing file, terminating program.\n");
        exit(-1);
    }; // Close and reopen after delay so we get an updated version
    sleep(delay);
    FILE *usage_updated = fopen("/proc/stat", "r");

    if(usage_updated == NULL){
        perror("Error opening file, terminating program.\n");
        exit(-1);
    }

    fgets(temp, 1000, usage_updated);

    for (int i = 0; i < 1000; i++)
    {
        if (!isdigit(temp[i]) && temp[i] != '\0')
        { // Replace all non digit characters with whitespace so we can use scanf
            temp[i] = ' ';
        }
    }

    sscanf(temp, " %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld", &user_time, &nice_time, &system_time, &idle_time, &iowait_time, &irq_time, &softirq_time, &steal_time, &guest_time, &guest_nice_time);

    total_time2 = user_time + nice_time + system_time + idle_time + iowait_time + irq_time + softirq_time;
    total_idle_time2 = idle_time;
    total_usage_time2 = total_time2 - total_idle_time2;

    long long total_usage_dif = total_usage_time2 - total_usage_time;
    long long total_time_dif = total_time2 - total_time;

    float cpu_usage = (float)total_usage_dif / total_time_dif; // Same cpu usage formula as explained above
    cpu_usage *= 100;

    if(fclose(usage_updated) != 0 || fclose(coreinfo) != 0){
        perror("Error closing file, terminating program.\n");
        exit(-1);
    }
    

    return cpu_usage;
}

void userInfo(int delay, int iteration, int samples)
{

    setutent(); // setutent() opens the utmp.h file, getutent() reads a line of the utmp.h file and stores the data represented from the line in the struct pointed to by user_data
    struct utmp *user_data;
    printf("\n---------------------------------------\n\n");
    printf("### Sessions/users ###\n");

    while ((user_data = getutent()) != NULL)
    { // If there are no more lines left in the utmp file, end the loop
        if (user_data->ut_type == USER_PROCESS)
        { // If the line type is a user entry, print desired information about it
            printf("%s    %s    %s\n", user_data->ut_user, user_data->ut_line, user_data->ut_host);
        }
    }
    printf("---------------------------------------\n");
    endutent(); // Close the file pointer
    if(iteration+1 == samples) return;
    sleep(delay);
}



