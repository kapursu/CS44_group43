//Supriya Kapur and Kenon Kahoano
//CS 444

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

//pthread and mutex vars
pthread_mutex_t mymutex;
pthread_cond_t consumer_condition;
pthread_cond_t producer_condition;
int location;
int num_data_points;
int i; 

//get random num will genrate num uses asm code in c!//
//Function code partly from:
//https://www.codeproject.com/Articles/15971/Using-Inline-Assembly-in-C-C
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

//This struct will store the points created by the producer//
struct data_point{
    	int value;
    	int sleep_time;
	bool added_to_buf;
};

//This buffer will hold the items and have fifo implementation// 
struct buffer{
	bool open;
	struct local* first_point;
	int num;
	int datapoint; 
	int space_available;
	int star_at_zer;
};

//Struct to help with fifo implementation//
struct local{
	struct data_point* data;
	struct local* new_data_point;
};

//Helper function for producer adding datapoint//
struct data_point* producer_producing(struct data_point* data){
	data->value = get_random_num()%10;
	//https://stackoverflow.com/questions/1202687/how-do-i-get-a-specific-range-of-numbers-from-rand
	data->sleep_time = (get_random_num()%8)+2;
	return data; 
}

//CHeck if there is space in buffer to produce another item//
bool space_available(struct buffer* buf){
	bool space;
	if(buf->num >= 31)
		space = false;
	else 
		space = true;
	return space;
} 

//CHeck if there are datapoints for the consumer to take//
bool data_point_to_consume(struct buffer* buf){
	bool data_points_2_take;
	if(buf->num < 1)
		data_points_2_take = false;
	else
		data_points_2_take = true; 

	return data_points_2_take;
}
	
//COnsumer main functionality//
void* consumer(void* arg)
{
    	struct buffer* buf = (struct buffer *) arg;
	int i;	
	//run loop 10 times 
   	for(i = 0; i < RUN_FOR_THIS_MANY_TIMES;){
		//lock the mutex
        	pthread_mutex_lock(&mymutex);

		//while there is no data points in the buffer, wait condition signaled
        	while(!data_point_to_consume(buf)){
           		pthread_cond_wait(&consumer_condition, &mymutex);
        	}
	
		//Consumer doing work: it will take the first point, update the head of the buffer
		//The consumer will take the first point from the buffer, decrease the number of items
		//in the buffer, update the head of the buffer, and free the old node.
        	struct local* mylocal = buf->first_point;
        	buf->first_point = buf->first_point->new_data_point;
        	buf->num--;
		buf->space_available++;
		buf->datapoint--;
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

//main producer functionality
void* producer(void* arg){
	struct buffer* buf = (struct buffer *) arg;
	int i; 
	//run loop ten times
	for(i = 0; i < RUN_FOR_THIS_MANY_TIMES;){
		//lock the mutex
        	pthread_mutex_lock(&mymutex);
		//IF the buffer at capacity (32 spaces), then signal wait condition
        	while(!space_available(buf)){
            		pthread_cond_wait(&producer_condition, &mymutex);
        	}
		
		//create new data point by creating new struct instance and 
		//calling producing function to fill it with data
       		struct data_point* data = malloc(sizeof(struct data_point));
        	data = producer_producing(data);
		buf->datapoint++;
		//Add the new data point to buffer by creating a new node and 
		//placing at the top of the buffer and increasing number in buffer
        	struct local* new_local = malloc(sizeof(struct local));
        	new_local->data = data;
        	new_local->new_data_point = buf->first_point;
        	buf->first_point = new_local;
        	buf->num++;
		buf->space_available--;
		
		//unlock mutex and waittime
		printf("Producing data_point. Buffer contains %d items \n",buf->num);
		pthread_cond_signal(&consumer_condition);
	        pthread_mutex_unlock(&mymutex);
		//https://stackoverflow.com/questions/1202687/how-do-i-get-a-specific-range-of-numbers-from-rand
	        sleep((get_random_num()%5) + 3);
        }	
	pthread_exit(0);
}

//Code for producing pthreads_mutex and pthread_cond from: http:
////cis.poly.edu/
//https://www.youtube.com/watch?v=1ks-oMotUjc
int main(int argc, char** argv){
	srand(time(NULL));
        
	//User must enter a.out followed by the number of threads//
	if(argc < 2){
                printf("Please enter the number of threads when running the program \n");
                exit(-1);
        }

	//Get the number of threads from user and initialize buffer and pointers//
	int num_args = atoi(argv[1]);
	int i;
        struct buffer buf;
        buf.first_point = NULL;
	buf.num = 0; 
	buf.space_available = 32;
	buf.datapoint = 0; 
        
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
	pthread_mutex_destroy(&mymutex); 
	pthread_cond_destroy(&consumer_condition);
	pthread_cond_destroy(&producer_condition);
	return 0;
}

