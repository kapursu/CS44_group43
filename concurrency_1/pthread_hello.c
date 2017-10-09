#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "mt19937ar.c"

struct args{
	long tid;
	long sleep_time;
};

void *hello(void *tid)
{
	struct args *a = (struct args*)tid;
	sleep(a->sleep_time);
	return (void*)printf("Hello from thread %ld! I did %ld units of work!\n", a->tid, a->sleep_time);
}

int main(int argc, char **argv)
{
	unsigned long init[4] = {0x123, 0x234, 0x345, 0x456};
	unsigned long length = 4;
	init_by_array(init, length);


	pthread_t threads[atoi(argv[1])];
	struct args a[atoi(argv[1])];

	for(long i = 0; i < atoi(argv[1]); ++i){

		/* int pthread_create(pthread_t *thread, const pthread_attr_t *attr, */
		/*                    void *(*start_routine) (void *), void *arg); */

		a[i].tid = i;
		a[i].sleep_time = genrand_int32() % 10;

		pthread_create(&(threads[i]),
		               NULL,
		               hello,
		               (void*)&a[i]);
	}

	for(long i = 0; i < atoi(argv[1]); ++i){
		pthread_join(threads[i], NULL);
	}
	
	return 0;
}

