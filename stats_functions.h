#ifndef stats_functions_h
#define stats_functions_h


void sysInfo();
float quick_cpu(int delay);
float quick_mem(char *placeholder, int graphics);
float cpuInfo(float previous, int delay, char* placeholder, int flag, int iteration, int samples, int sequential_flag, int graphics_flag);
void userInfo(int delay, int iteration, int samples);
void memoryInfo(char* previous, char* placeholder, int graphics_flag, float old_usage, int* mem_pipe);

#endif