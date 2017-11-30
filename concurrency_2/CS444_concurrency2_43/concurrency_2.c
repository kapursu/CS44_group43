#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "mt19937ar.c"
#include <string.h>

pthread_mutex_t forks[5];
pthread_mutex_t mymutex;

unsigned int eax;
unsigned int ebx;
unsigned int ecx;
unsigned int edx;
unsigned int r;

/* Function: get_random_num
 * Uses asm code in c to get random num
 * Code taken from:https://www.codeproject.com/Articles/15971/Using-Inline-Assembly-in-C-C
 */
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

/* Struct: phi
 * Holds all information about each philosopher
 */
struct phi{
	int table_location;
	int status; //eating, waiting, thinking, etc.
	char name[10];
};

/* Function: think
 * Uses get_random_num function to get a time between 1 and 20 seconds
 * for the person to think. Locks the overseeing mutex during this time
 */
void think(struct phi *person){
	int eat_time = (get_random_num() %20)+1;
	pthread_mutex_lock(&mymutex); 
	printf("%s is going to think for %d seconds. \n", person->name, eat_time); 	
	pthread_mutex_unlock(&mymutex); 
	sleep(eat_time);
}

/* Function: left
 * find the correct fork on the left of a circular table
 * Takes the location of the person
 */
int left(int i){
	return i; 
}

/* Function: right
 * find the correct fork on the right of a circular table
 * Takes the location of the person 
 */
int right(int i){
	return ((i + 1) % 5);
}

/* Function: get_forks
 * Takes philosopher struct and finds the forks held by the peron by
 * using the right() and left() function, momentarily locks the overseeing
 * mutex and unlocks the fork mutex
 */
void get_forks(struct phi *person){
	int location = person->table_location;
	int left_fork = left(location); 
	int right_fork = right(location); 

	
	pthread_mutex_lock(&mymutex); 
	//pthread_mutex_lock(&forks[left_fork]); 
	printf("%s is picking up fork %d. \n", person->name, left_fork); 	
	pthread_mutex_unlock(&mymutex); 
	pthread_mutex_lock(&forks[left_fork]); 

	pthread_mutex_lock(&mymutex); 
	//pthread_mutex_lock(&forks[right_fork]); 
	printf("%s is picking up fork %d.  \n", person->name, right_fork); 	
	pthread_mutex_unlock(&mymutex); 
	pthread_mutex_lock(&forks[right_fork]); 
	 

}

/* Function: eat
 * Uses get_random_num function to get a time between 9 and 2 seconds 
 * for the person to eat. Locks the overseeing mutex during this time
 */
void eat(struct phi *person){
	int sleep_time = (get_random_num()%9)+2; 
	pthread_mutex_lock(&mymutex); 
	printf("%s is going to eat for %d seconds. \n", person->name, sleep_time); 	
	pthread_mutex_unlock(&mymutex); 
	sleep(sleep_time); 
}

/* Function: put_forks()
 * Takes the philosopher struct, find the correct right and left
 * fork using the right() and left() function
 * Locks the overseeing mutex and and the correct forks
 */
void put_forks(struct phi *person){
	int location = person->table_location; 
	int left_fork = left(location); 
	int right_fork = right(location); 

	pthread_mutex_lock(&mymutex); 
	printf("%s is putting down their forks. \n", person->name); 	
	pthread_mutex_unlock(&mymutex); 
	pthread_mutex_unlock(&forks[left_fork]); 
	pthread_mutex_unlock(&forks[right_fork]); 
}


/* Function: dining_problem
 * Contains all function calls for dining problem
 * Code taken directly from assignment page
 */ 
void *dining_problem(void *philosopher){

	struct phi *person = (struct phi *)philosopher; 

	while(1){
		think(person);
		get_forks(person);
		eat(person);
		put_forks(person);
	}
}

/* Function: Main()
 * Initilize all philosophers (names, place at table)
 * Initilize mutexs and threads
 * Call process and join with pthread_join
 * Clean up by destroying all mutex
 */
int main(){ 
	//initilaize philosophers
	struct phi *people; 
	people = malloc(sizeof(struct phi)*5); 
	
	//asign table location
	people[0].table_location = 0;
	strcpy(people[0].name, "Aristotle");
 
	people[1].table_location = 1;
	strcpy(people[1].name, "Kant"); 

	people[2].table_location = 2;
	strcpy(people[2].name, "Plato");

	people[3].table_location = 3;
	strcpy(people[3].name, "Confucius");
 
	people[4].table_location = 4;
	strcpy(people[4].name, "Hume"); 
	
	pthread_mutex_init(&mymutex, NULL);
	
	int i; 

	for (i = 0; i < 5; i++){
		pthread_mutex_init(&forks[i], NULL);
	}

	pthread_t thread[5];
	
	for (i = 0; i < 5; i++){
		pthread_create(&thread[i], NULL, dining_problem, &people[i]);
	}
	
	for (i = 0; i < 5; i++){
		pthread_join(thread[i], NULL);
	}

	pthread_mutex_destroy(&mymutex); 
	
	for (i = 0; i<5; i++){
		pthread_mutex_destroy(&forks[i]);
	}	
	
	return 0;
}
