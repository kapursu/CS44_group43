#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "mt19937ar.c"
#include "time.h"
#define RUN_FOR_THIS_MANY_TIMES 10
//global variables
unsigned int eax;
unsigned int ebx;
unsigned int ecx;
unsigned int edx;
unsigned int r;

pthread_mutex_t mymutex;
pthread_cond_t consumer_condition;
pthread_cond_t producer_condition;
int location;
int num_data_points;
int i; 

unsigned int get_random_num(void){
        char vendor[13];

        eax = 0x01;

        __asm__ __volatile__("cpuid;" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));

        if(ecx & 0x40000000){
                unsigned int val;
                __asm__ __volatile__ ("rdrand %0" : "=r" (val));
                return val;

        }else{
                unsigned int val;
                val = genrand_int32();

                return val;

        }
}

struct data_point* producer_producing(struct data_point* data){
	data->value = get_random_num()%10;
	data->sleep_time = (get_random_num()%3)+7;
	return data; 
}

bool space_available(struct buffer* buf){
	bool space;
	if(buf->num >= 31)
		space = false;
	else 
		space = true;
	return space; 

struct buffer{
	struct local* first_point;
	int num;
};

struct local{
	struct data_point* data;
	struct local* new_data_point;
};

struct data_point{
    	int value;
    	int sleep_time;
};

bool data_point_to_consume(struct buffer* buf){
	bool data_points_2_take;
	if(buf->num < 1)
		data_points_2_take = false;
	else
		data_points_2_take = true; 

	return data_points_2_take;
}
	
void* consumer(void* arg)
{
    	struct buffer* buf = (struct buffer *) arg;
	int i; 
   	for(i = 0; i < RUN_FOR_THIS_MANY_TIMES;){
        	pthread_mutex_lock(&mymutex);
        	while(!data_point_to_consume(buf)){
           		pthread_cond_wait(&consumer_condition, &mymutex);
        	}

        	struct local* mylocal = buf->first_point;
        	buf->first_point = buf->first_point->new_data_point;
        	buf->num--;
        	struct data_point* temp = mylocal->data;
		free(mylocal);
		struct data_point* data = temp;
       	        printf("Consuming Item:  %d\n", data->value);
		//unlock mutex and wait time
		pthread_cond_signal(&producer_condition);
        	pthread_mutex_unlock(&mymutex);
        	sleep(data->sleep_time);
        	free(data);
    	}
    	pthread_exit(0);
}

struct data_point* producer_producing(struct data_point* data){
	data->value = get_random_num()%10;
	data->sleep_time = (get_random_num()%3)+7;
	return data; 
}

bool space_available(struct buffer* buf){
	bool space;
	if(buf->num >= 31)
		space = false;
	else 
		space = true;
	return space; 
}

void* producer(void* arg){
	struct buffer* buf = (struct buffer *) arg;
	int i; 
	for(i = 0; i < RUN_FOR_THIS_MANY_TIMES;){
        	pthread_mutex_lock(&mymutex);
        	while(!space_available(buf)){
            		pthread_cond_wait(&producer_condition, &mymutex);
        	}
       		struct data_point* data = malloc(sizeof(struct data_point));
        	data = producer_producing(data);
	
        	struct local* new_local = malloc(sizeof(struct local));
        	new_local->data = data;
        	new_local->new_data_point = buf->first_point;
        	buf->first_point = new_local;
        	buf->num++;
		

		//unlock mutex and waittime
		printf("Producing data_point. Buffer contains %d items \n",buf->num);
		pthread_cond_signal(&consumer_condition);
	        pthread_mutex_unlock(&mymutex);
	        sleep((get_random_num()%5) + 3);
        }	
	pthread_exit(0);
}


int main(int argc, char** argv){
	srand(time(NULL));
        
	//User must enter a.out followed by the number of threads//
	if(argc < 2){
                printf(" AYYYPlease enter the number of threads when running the program \n");
                exit(-1);
        }

	//Get the number of threads from user and initialize buffer and pointers//
	int num_args = atoi(argv[1]);
	int i;
        struct buffer buf;
        buf.first_point = NULL;
	buf.num = 0; 
        
	//Init mutex and threads//
       	pthread_t tid[num_args*2];
        pthread_mutex_init(&mymutex, NULL);
        pthread_cond_init(&consumer_condition, NULL); 
	pthread_cond_init(&producer_condition, NULL);
	pthread_attr_t attr; 
	pthread_attr_init (&attr);
	
	//create pthreads//
        for(i=0; i < (num_args); i++){
                pthread_create(&tid[i], &attr, producer, &buf);
        }

        for(i=(num_args); i <num_args*2; i++){
               pthread_create(&tid[i], &attr, consumer,&buf);
        }

        for (i = 0; i <num_args; i++){
                pthread_join(tid[i], NULL);
        }
	
	//clean up//
	//pthread_mutex_destroy(&buf); 
	return 0;
}

