#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

int main(void){
    long free_mem = syscall(359);
    long total_mem = syscall(360);

    float actual_mem = ((float)free_mem/(float)total_mem);

    printf("Free memory: %l\n", free_mem);
    printf("Total memory: %l\n", total_mem);
    printf("Actual memory: %f\n", actual_mem);
    
    return 0;
}
