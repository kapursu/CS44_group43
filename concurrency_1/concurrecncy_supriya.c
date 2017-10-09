#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "mt19937ar.c"

//#include "mt.h"


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
	int buffer_size; 
	int spot1;
	struct item* data_point;
	struct local* top_data_point;  
};

struct local{
	struct local* spot_in_buf; 
	struct item* data;
};

struct item{
	int value;
	int wait_time;
};
struct spaces {
	bool available; 
}; 






void print_stack(struct buffer* buf){
    printf("%d items on stack: ", buf->spot1);
    int i;
    struct local* start = buf->top_data_point;
    for(i = 0; i < buf->spot1; i++){
        printf("%d,",start->data->value);
        start = start->spot_in_buf;
    }
    printf("\n");
}


//See if there is space in the buffer
//bool is_there_space(struct buffer* buf, struct spaces* space){
//	if(buf->spot < 30)
//		space->available = false;
//	return space->available; 
//}
void* consumer_func(void* input){
	int i;
	struct buffer* buf = (struct buffer *) input;
	for(i=0; i <10; i++){
		pthread_mutex_lock(&mymutex);
		while(num_items < 1){
			pthread_mutex_unlock(&mymutex);
			sleep(1); 
			pthread_mutex_lock(&mymutex);
			//pthread_cond_wait(&consumer_condition, &mutex);
		}
		//do work
		//struct local* new_top = buf->top_data_point; 
		//buf->top_data_point = buf->top_data_point->spot_in_buf;
		//num_items--; 
	        //struct item* data = new_top->data;
		//free(new_top); 
		struct item* data = pop(buf);
		sleep(data->wait_time);
		printf("Consuming Item: %d\n", data->value);  
		//pthread_cond_signal(&producer_condition); 
		pthread_mutex_unlock(&mymutex); 
	}
	pthread_exit(0);
}

struct item* pop(struct buffer* buf){
    if(buf->spot1 > 0){
        struct local* node = buf->top_data_point;
        buf->top_data_point = buf->top_data_point->spot_in_buf;
        buf->spot1--;
        struct item* temp = node->data;
        free(node);
        return temp;
    }
    return NULL;
}

//void push(struct stack* mystack, struct item* myitem){
//    struct node* new_node = malloc(sizeof(struct node));
//    new_node->myitem = myitem;
//    new_node->next = mystack->head;
//    mystack->head = new_node;
//    mystack->num++;
//}



void* producer_func(void* input){
	int i, spot1;
	struct buffer* buf = (struct buffer *) input; 
	for(i = 0; i < 10; i++){
		pthread_mutex_lock(&mymutex);
		spot1 = buf->spot1;
		while(spot1 > 31){
			//pthread_cond_wait(&producer_condition, &mutex);
			pthread_mutex_unlock(&mymutex);
			sleep(1);
			pthread_mutex_lock(&mymutex); 
			spot1 = buf->spot1;
		}
		//do work
		struct item* data = malloc(sizeof(struct item)); 
		data->value = get_random_num()%1000;
		data->wait_time= ((get_random_num()%3)+7);
		sleep((get_random_num()%2)+9); 
		struct local* spot = malloc(sizeof(struct local)); 
		spot->data = data; 
		spot->spot_in_buf = buf->top_data_point;
		buf->top_data_point = spot;
		buf->spot1++;
		print_stack(buf); 
	//	pthread_cond_signal(&consumer_condition); 
		pthread_mutex_unlock(&mymutex);
	}
	pthread_exit(0);
		
}
int main(int argc, char** argv){

	if(argc < 2){
		printf("Usage: %s <number of threads>\n", argv[0]);
		exit(-1);
	}


	struct buffer buf;
	buf.buffer_size = 32; 
	buf.top_data_point = NULL; 
	struct spaces space = {true}; 

	int num_args = atoi(argv[1]);
	location = 0; 
	num_items = 0;
	pthread_t tid[num_args*2];
	pthread_mutex_init(&mymutex, NULL);
	//pthread_cond_init(&consumer_condition, NULL); 
	//pthread_cond_init(&producer_condition, NULL);
	int i;
	
	for(i=0; i < (num_args); i++){
		pthread_attr_t attr;
		pthread_attr_init(&attr); 
		pthread_create(&tid[i], &attr, producer_func, &mymutex);
	}

	for(i=(num_args); i <num_args*2; i++){
		pthread_attr_t attr;
		pthread_attr_init(&attr); 
		pthread_create(&tid[i], &attr, consumer_func,&mymutex);
	}

	for (i = 0; i <num_args; i++){
		pthread_join(tid[i], NULL); 
	}	
 
}

