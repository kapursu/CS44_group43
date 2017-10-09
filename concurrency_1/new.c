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
int num_items;
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


struct buffer{
	struct node* first_point;
	int num;
};

struct node{
	struct item* data;
	struct node* next;
};

struct item{
    	int value;
    	int con_wait;
};

bool item_to_consume(struct buffer* buf){
	bool items_2_take;
	if(buf->num < 1)
		items_2_take = false;
	else
		items_2_take = true; 

	return items_2_take;
}
	
void* consumer(void* arg)
{
    	struct buffer* buf = (struct buffer *) arg;
	int i; 
   	for(i = 0; i < RUN_FOR_THIS_MANY_TIMES;){
        	pthread_mutex_lock(&mymutex);
        	while(!item_to_consume(buf)){
           		pthread_cond_wait(&consumer_condition, &mymutex);
        	}

        	struct node* mynode = buf->first_point;
        	buf->first_point = buf->first_point->next;
        	buf->num--;
        	struct item* temp = mynode->data;
		free(mynode);
		struct item* data = temp;
       	        printf("Consuming Item:  %d\n", data->value);
		//unlock mutex and wait time
		pthread_cond_signal(&producer_condition);
        	pthread_mutex_unlock(&mymutex);
        	sleep(data->con_wait);
        	free(data);
    	}
    	pthread_exit(0);
}

struct item* producer_producing(struct item* data){
	data->value = get_random_num()%10;
	data->con_wait = (get_random_num()%3)+7;
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
       		struct item* data = malloc(sizeof(struct item));
        	data = producer_producing(data);
	
        	struct node* new_node = malloc(sizeof(struct node));
        	new_node->data = data;
        	new_node->next = buf->first_point;
        	buf->first_point = new_node;
        	buf->num++;
		

		//unlock mutex and waittime
		printf("Producing item. Buffer contains %d items \n",buf->num);
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
                printf("Please enter the number of threads when running the program \n");
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

