#include <stdio.h>
#include "buffer.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_mutex_t produceMonitor;
pthread_mutex_t consumerMonitor;
pthread_mutex_t produceMutex;
pthread_mutex_t consumerMutex;

sem_t full, empty;

buffer_item buffer[BUFFER_SIZE];
buffer_item item;
buffer_item consumeItem;

int cnt;
int produceCheck = 0;
int consumerCheck = 0;

pthread_attr_t attr;

int insert_item(buffer_item item1){
    /* insert item into buffer
     return 0 if successful, otherwise
     return -1 indicating an error condition */
    if(cnt < BUFFER_SIZE) {
        buffer[cnt] = item1;
        cnt++;
        return 0;
    }
    else return -1;
}

int remove_item(buffer_item *item){
    /* remove an object from buffer placing it in item
     return 0 if successful, otherwise
     return -1 indicating an error condition */
    if(cnt > 0) {
        *item = buffer[(cnt-1)];
        cnt--;
        return 0;
    }
    else return -1;
}

void Initializebuffer() {
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&produceMonitor, NULL);
    pthread_mutex_init(&consumerMonitor, NULL);
    pthread_mutex_init(&produceMutex, NULL);
    pthread_mutex_init(&consumerMutex, NULL);
    
    sem_init(&full, 0, 0);
    
    sem_init(&empty, 0, BUFFER_SIZE);
    
    pthread_attr_init(&attr);
    
    cnt = 0;
}

void *monitorProducer(void* param) {
    
    while(1) {
        sem_wait(&empty);
        
        pthread_mutex_lock(&produceMutex);
        pthread_mutex_lock(&produceMonitor);
        
        if(item <= 50) {
            printf("Produce Monitor : %d 생성 가능\n", item);
            produceCheck = 1;
        }
        else {
            printf("Produce Monitor : %d 값이 50보다 큼.\n", item);
            produceCheck = 0;
        }
        pthread_mutex_unlock(&produceMonitor);
        
        //pthread_mutex_unlock(&produceMutex);
        sem_post(&full);
    }
}

void *produce(void* param) {
    int i;
    
    while(1){
        /* sleep for a random period of time */
        int sleepTime = rand() / 100000000;
        //printf("producer thread\n");
        sleep(sleepTime);
        /* generate a random number between 1 and 100 */
        
        pthread_mutex_lock(&mutex);
        
        item = (rand() % 100) + 1;
        
        pthread_mutex_unlock(&produceMutex);
        sleep(1);
        pthread_mutex_lock(&produceMonitor);
        
        if(produceCheck == 1){
            if(insert_item(item))
                fprintf(stderr, "Produce : 오류\n");
            else {
                printf("Produce : %d 생성\n", item);
                printf("buffer_item : [");
                for(i = 0; i < cnt; i++)
                    printf(" %d ", buffer[i] );
                printf("] \n");
            }
        }
        else {
            printf("Produce : reject %d\n", item);
        }
        
        produceCheck = 0;
        
        pthread_mutex_unlock(&produceMonitor);
        pthread_mutex_unlock(&mutex);
        
    }
}

void *monitorConsumer(void* param) {
    while(1) {
        sem_wait(&full);
        pthread_mutex_lock(&consumerMutex);
        pthread_mutex_lock(&consumerMonitor);
        
        if(consumeItem <= 25) {
            printf("Consumer monitor : %d 소비 가능\n", consumeItem);
            consumerCheck = 1;
        }
        else {
            printf("Consumer Monitor : %d 값이 25보다 큼.\n", consumeItem);
            consumerCheck = 0;
        }
        pthread_mutex_unlock(&consumerMonitor);
        
        sem_post(&empty);
    }
}

void *consumer(void* param) {
    int i;
    
    while(1){
        /* sleep for a random period of time */
        int sleepTime = rand() / 100000000;
        //printf("consumer thread\n");
        sleep(sleepTime);
        
        
        pthread_mutex_lock(&mutex);
        
        if(cnt > 0){
            consumeItem = buffer[(cnt-1)];
            
            pthread_mutex_unlock(&consumerMutex);
            sleep(1);
            pthread_mutex_lock(&consumerMonitor);
            
            if(consumerCheck == 1){
                if(remove_item(&item))
                    fprintf(stderr, "Consumer : 오류\n");
                else
                    printf("Consumer : 소비 %d\n", item);
                printf("buffer_item : [");
                for(i = 0; i < cnt; i++)
                    printf(" %d ", buffer[i] );
                printf("] \n");
                
            }
            else {
                buffer[(cnt-1)] = buffer[(cnt-1)] / 2;
                printf("Consumer : %d 값을 dlvide 2\n", consumeItem);
                printf("buffer_item : [");
                for(i = 0; i < cnt; i++)
                    printf(" %d ", buffer[i] );
                printf("] \n");
            }
        }
        pthread_mutex_unlock(&consumerMonitor);
        pthread_mutex_unlock(&mutex);
        
    }
    
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        fputs("ERROR, argc INPUT\n", stderr);
        exit(1);
    }
    
    int i;
    int sleepTime = atoi(argv[1]);
    int producerNumber = atoi(argv[2]);
    int consumerNumber = atoi(argv[3]);
    
    Initializebuffer();
    
    pthread_mutex_lock(&produceMutex);
    pthread_mutex_lock(&consumerMutex);
    
    pthread_t* produceThread;
    pthread_t* consumerThread;
    pthread_t monitorConsumerThread;
    pthread_t monitorProduceThread;
    
    produceThread = (pthread_t *)malloc(sizeof(pthread_t)*producerNumber);
    consumerThread = (pthread_t *)malloc(sizeof(pthread_t)*consumerNumber);
    
    for(i = 0; i < producerNumber; i++) {
        pthread_create(&produceThread[i], &attr, produce ,NULL);
    }
    
    for(i = 0; i < consumerNumber; i++) {
        pthread_create(&consumerThread[i], &attr, consumer ,NULL);
    }
    
    pthread_create(&monitorProduceThread, &attr, monitorProducer, NULL);
    pthread_create(&monitorConsumerThread, &attr, monitorConsumer, NULL);
    
    sleep(sleepTime);
    
    free(produceThread);
    free(consumerThread);
    
    printf("프로그램 종료\n");
    exit(0);
}
