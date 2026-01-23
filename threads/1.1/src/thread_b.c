#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_THREADS 5

void *mythread(void __attribute__((unused)) *arg) {
    printf("mythread [%d %d %d]: Hello from mythread!\n", getpid(), getppid(), gettid());
    return NULL;
}

int main() {
    pthread_t tid[NUM_THREADS];
    int err;

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    for (int i = 0; i < NUM_THREADS; i++) {
        err = pthread_create(&tid[i], NULL, mythread, NULL);
        if (err) {
            printf("main: pthread_create() failed for thread %d: %s\n", i, strerror(err));
            for (int j = 0; j < i; j++) {
                pthread_join(tid[j], NULL);
            }
            return -1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        err = pthread_join(tid[i], NULL);
        if (err) {
            printf("main: pthread_join() failed for thread %d: %s\n", i, strerror(err));
            return -1;
        }
    }

    return 0;
}
