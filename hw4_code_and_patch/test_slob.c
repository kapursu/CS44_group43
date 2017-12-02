#include <linux/module.h>
#include <linux/random.h>
#include <linux/slob.h>
#include <sys/syscall.h>
#include <sys/types.h>

int main(void){
    long free = syscall(353);
    long total = syscall(354);
    float actual = ((float) free/ (float) total);

    printf("Free space: %lu\n", free);
    printf("Total space: %lu\n", total);
    printf("Actual Memory usage: %f\n", actual);

    return 0;
}
