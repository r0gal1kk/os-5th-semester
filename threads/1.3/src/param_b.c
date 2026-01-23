#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct {
    int num;
    char *str;
} my_struct;

void *mythread(void *arg) {
    my_struct *data = (my_struct *)arg;
    while (1) printf("mythread: num = %d, str = %s\n", data->num, data->str);
    sleep(5);
    printf("mythread: after sleep - num = %d, str = %s\n", data->num, data->str);
    free(data);
    return NULL;
}

int main() {
    pthread_t tid;
    int err;
    pthread_attr_t attr;

    my_struct *data = malloc(sizeof(my_struct));
    if (!data) {
        printf("main: malloc failed\n");
        return -1;
    }
    data->num = 42;
    data->str = "hello world";

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    err = pthread_create(&tid, &attr, mythread, data);
    if (err) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        free(data);
        return -1;
    }

    pthread_attr_destroy(&attr);
    sleep(1);
    printf("main: exiting early\n");
    pthread_exit(NULL);
}