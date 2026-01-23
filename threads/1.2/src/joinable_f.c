#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

void *mythread(void __attribute__((unused)) *arg) {
    printf("mythread [%d %d %d]: Hello and goodbye!\n", getpid(), getppid(), gettid());
    return NULL;
}

int main() {
    pthread_t tid;
    pthread_attr_t attr;
    int err;

    printf("main [%d %d %d]: Starting loop...\n", getpid(), getppid(), gettid());

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    while (1) {
        err = pthread_create(&tid, &attr, mythread, NULL);
        if (err) {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            break;
        }
    }

    pthread_attr_destroy(&attr);
    return 0;
}