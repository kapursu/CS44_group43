#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <immintrin.h>
#include <unistd.h>
#include "mt19937ar.c"
//#include <sys/queue.h>

long get_rand();
pthread_mutex_t thestack;

struct stack{
    struct node* head;
    int num;
};

struct node{
    struct item* myitem;
    struct node* next;
};

struct item{
    int value;
    int con_wait;
};

void print_stack(struct stack* mystack){
    printf("%d items on stack: ", mystack->num);
    int i;
    struct node* start = mystack->head;
    for(i = 0; i < mystack->num; i++){
        printf("%d,",start->myitem->value);
        start = start->next;
    }
    printf("\n");
}

struct item* pop(struct stack* mystack){
    if(mystack->num > 0){
        struct node* mynode = mystack->head;
        mystack->head = mystack->head->next;
        mystack->num--;
        struct item* temp = mynode->myitem;
        free(mynode);
        return temp;
    }
    return NULL;
}

void push(struct stack* mystack, struct item* myitem){
    struct node* new_node = malloc(sizeof(struct node));
    new_node->myitem = myitem;
    new_node->next = mystack->head;
    mystack->head = new_node;
    mystack->num++;
}

void* consumer(void* arg)
{
    struct stack* mystack = (struct stack *) arg;
    int i, num;
    for(i = 0; i < 100;){
        pthread_mutex_lock(&thestack);
        num = mystack->num;
        while(num < 1){
            pthread_mutex_unlock(&thestack);
            sleep(1);
            pthread_mutex_lock(&thestack);
            num = mystack->num;
        }
        struct item* myitem = pop(mystack);
        printf("Popped %d\n", myitem->value);
        pthread_mutex_unlock(&thestack);
        sleep(myitem->con_wait);
        free(myitem);
    }
    pthread_exit(0);
}

void* producer(void* arg)
{
    struct stack* mystack = (struct stack *) arg;
    int i, num;
    for(i = 0; i < 100;){
        pthread_mutex_lock(&thestack);
        num = mystack->num;
        while(num > 31){
            pthread_mutex_unlock(&thestack);
            sleep(1);
            pthread_mutex_lock(&thestack);
            num = mystack->num;
        }
        struct item* myitem = malloc(sizeof(struct item));
        myitem->value = get_rand()%10000;
        myitem->con_wait = (get_rand()%3)+7;
        push(mystack, myitem);
        print_stack(mystack);
        pthread_mutex_unlock(&thestack);
        sleep((get_rand()%5) + 3);
    }
    pthread_exit(0);
}


int main(int argc, char**argv)
{
    struct stack mystack;
    mystack.head = NULL;
    mystack.num = 0;
    if(argc != 2) {
        printf("Usage: %s <# of threads>\n", argv[0]);
        exit(-1);
    }
    int num_threads = atoi(argv[1]);

    pthread_t tids[num_threads*2];

    pthread_mutex_init(&thestack, NULL);

    
    int i = 0;
    for(i = 0; i < num_threads; i++){
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tids[i], &attr, producer, &mystack);
    }
    for(i = num_threads; i < num_threads*2; i++){
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tids[i], &attr, consumer, &mystack);
    }
    //Wait until thread is done
    for(i = 0; i < num_threads; i++){
        pthread_join(tids[i], NULL);
    }
}

long get_rand(){
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;

    char vendor[13];

    eax = 0x01;

    __asm__ __volatile__(
            "cpuid;"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(eax)
            );

    if(ecx & 0x40000000){
        //use rdrand
        unsigned int rnd;
        __asm__ __volatile__ ("rdrand %0" : "=r" (rnd));
        return rnd;
    }
    else{
        //use mt19937
        return genrand_int32();
    }
    return 0;
}
