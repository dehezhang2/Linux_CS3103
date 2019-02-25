#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <stdio.h>

using namespace std;

#define NUM_THREADS 5

void *PrintHello(void *threadid) {
   long tid= (long)threadid;
   printf("Hello World! Thread ID: %ld\n", tid);
   pthread_exit(NULL);
}

int main () {
   pthread_t threads[NUM_THREADS];
   int rc;
   int i;
   
   for( i = 0; i < NUM_THREADS; i++ ) {
      printf("main() : creating thread: %d\n", i);
      rc = pthread_create(&threads[i], NULL, PrintHello, (void*)i);
      
      if (rc) {
         printf("Error:unable to create thread: %d\n", rc);
         exit(-1);
      }
   }
   pthread_exit(NULL);
}
