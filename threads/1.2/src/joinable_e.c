#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

void *mythread(void __attribute__((unused)) *arg) {
    int err = pthread_detach(pthread_self());
    if (err) {
        printf("mythread: pthread_detach() failed: %s\n", strerror(err));
    }
    printf("mythread [%d %d %d]: Hello and goodbye!\n", getpid(), getppid(), gettid());
    return NULL;
}

int main() {
    pthread_t tid;
    int err;

    printf("main [%d %d %d]: Starting infinite loop...\n", getpid(), getppid(), gettid());

    while (1) {
        err = pthread_create(&tid, NULL, mythread, NULL);
        if (err) {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            break;
        }
    }

    return 0;
}