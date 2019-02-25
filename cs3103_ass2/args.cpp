#include <iostream>
#include <cstdlib>
#include <pthread.h>

using namespace std;

#define NUM_THREADS 5

typedef struct thread_data {
    int  thread_id;
    char *message;
} args;

void *PrintHello(void *threadarg) {
    args *my_data;
    my_data = (args *) threadarg;

    cout << "Thread ID : " << my_data->thread_id ;
    cout << " Message :" << my_data->message << endl;

    pthread_exit(NULL);
}

int main () {
    pthread_t threads[NUM_THREADS];
    args td[NUM_THREADS];
    int rc;
    int i;

    for( i = 0; i < NUM_THREADS; i++ ) {
        cout <<"main() : creating thread, " << i << endl;
        td[i].thread_id = i;
        td[i].message = "This is message";
        rc = pthread_create(&threads[i], NULL, PrintHello, (void *)&td[i]);
        pthread_join(threads[i],NULL);

        /* if(i==3)break; */
        if (rc) {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    pthread_exit(NULL);
}
