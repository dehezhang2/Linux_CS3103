#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <iostream>
#include "Queue.h"
using namespace std;

// use integer to simulate Token
#define Token int

// argument and mutex initialization
struct Thread_arg {
    Queue<Token>* buffer;
    int max_token;
    double flow_interval;
    Thread_arg(Queue<Token>* b, int m, double f) {
        buffer = b;
        max_token = m;
        flow_interval = f;
    }
};

// shared resources
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;
int flow_generate, fetched_token, dropped_token, pflow_generate;
int seq_num;

// semaphores
// The value of the empty is initialized as zero to block the pflow thread
// Server_permission is used to make sure server can only call pflow for one time
sem_t empty , server_permission;

// get random number
int getRand(int a, int b) {
    return rand() % (b - a + 1) + a;
}

// pflow thread
void *Pflow(void *threadarg){
    Thread_arg* my_data;
    my_data = (Thread_arg*)threadarg;
    const int max_token = my_data->max_token;
    Queue<Token>* buffer = my_data -> buffer;

    while(fetched_token + dropped_token < max_token){
        sem_wait(&empty);
        pthread_mutex_lock(&my_mutex);
        /*critical section*/
        int added_token = getRand(1, 5);
        pflow_generate += added_token;
        seq_num += added_token;

        for(int i = 0; i < added_token; i++) {
            if(dropped_token + fetched_token >= max_token)break;
            if(!buffer->push(seq_num)) {
                dropped_token++;
            }
        }
        printf("%3d(pflow)          %3d                    %3d\n", added_token, seq_num - 1, buffer->size());

        pthread_mutex_unlock(&my_mutex);
        sem_post(&server_permission);       // pflow has already respond to the request, and allows the server to continue;
    }
    pthread_exit(NULL);
}


// flow thread
void *Flow(void *threadarg) {
    Thread_arg* my_data;
    my_data = (Thread_arg*)threadarg;
    const int max_token = my_data->max_token;
    const double flow_interval = my_data->flow_interval;
    Queue<Token>* buffer = my_data -> buffer;

    while(fetched_token + dropped_token < max_token) {
        usleep((unsigned int)(flow_interval * 1e6));

        pthread_mutex_lock(&my_mutex);
            /*critical section*/
            int added_token = getRand(1, 10);
            flow_generate += added_token;
            seq_num += added_token;
            // use a for loop to simulate
            for(int i = 0; i < added_token; i++) {
                if(dropped_token + fetched_token >= max_token)break;
                if(!buffer->push(seq_num)) {
                    dropped_token++;
                }
            }
            printf("%3d(flow)          %3d                    %3d\n", added_token, seq_num - 1, buffer->size());
        pthread_mutex_unlock(&my_mutex);
    }
    pthread_exit(NULL);
}

// server thread
void *Server(void *threadarg) {
    Thread_arg* my_data;
    my_data = (Thread_arg*)threadarg;
    int max_token = my_data->max_token;
    Queue<Token>* buffer = my_data -> buffer;

    while(fetched_token + dropped_token < max_token) {
        usleep(2000000);

        pthread_mutex_lock(&my_mutex);
            /*critical section*/
            int deleted_token = getRand(1, 20);

            int cnt;
            for(cnt = 0; cnt < deleted_token; cnt++) {
                if(dropped_token + fetched_token >= max_token)break;
                if(!buffer->pop())break;
                fetched_token++;
            }
            printf("                                    %3d             %3d          %3d\n"
                   , buffer->size(), cnt, fetched_token);

            // when the queue is emptied by the server, the server will call p_flow 
            if(buffer -> size() == 0 && seq_num > 0){
                sem_post(&empty);
                pthread_mutex_unlock(&my_mutex);    // unlock the mutex to make sure the flow or pflow can execute as normal

                sem_wait(&server_permission);       // block the server until the pflow is finished
                continue;                           // make sure that the mutex will not be unlocked twice
            }

        pthread_mutex_unlock(&my_mutex);

    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if(argc != 3) {
        printf("Usage: $ ./Filename max_token flow_interval(only accept 2 variables)\n");
    } else {
        // initialization and create the argument needed by the threads
        fetched_token = dropped_token = flow_generate = pflow_generate = seq_num = 0;
        int max_token = atoi(argv[1]);
        double flow_interval = atof(argv[2]);
        sem_init(&empty,0,0);
        sem_init(&server_permission,0,0);

        Thread_arg arg(new Queue<Token>(), max_token, flow_interval);
        printf("Flow         Queue                                  Server\n");
        printf("Token added  Latest sequence number Current Length  Token served Total Token fetched\n");
        srand((unsigned)time(NULL));    // feed a seed

        // create 2 threads to do the simulation
        int record;
        pthread_t flow_thread, server_thread, pflow_thread;
        // start client thread at first
        record = pthread_create(&flow_thread, NULL, Flow, (void*)&arg);
        if (record) {
            printf("Error when creating client thread!\n");
            exit(-1);
        }

        record = pthread_create(&server_thread, NULL, Server, (void*)&arg);
        if (record) {
            printf("Error when creating server thread!\n");
            exit(-1);
        }

        record = pthread_create(&pflow_thread, NULL, Pflow, (void*)&arg);
        if(record){
            printf("Error when creating pflow thread!\n");
            exit(-1);
        }

        // wait for 2 thread to finish
        record = pthread_join(flow_thread, NULL);
        if (record) {
            printf("Error when joining flow thread!\n");
            exit(-1);
        }

        record = pthread_join(server_thread, NULL);
        if (record) {
            printf("Error when joining server thread!\n");
            exit(-1);
        }

        record = pthread_join(pflow_thread, NULL);
        if(record){
            printf("Error when joining pflow thread!\n");
            exit(-1);
        }

        printf("The total number of tokens that have been fetched by the server is %d.\n", fetched_token);
        printf("The total number of tokens that have been generated by the flow is %d.\n", flow_generate);
        printf("The total number of tokens that have been generated by the pflow is %d.\n", pflow_generate);
        printf("The total number of tokens that have been dropped by the queue is %d.\n", dropped_token);

    }
    return 0;
}
