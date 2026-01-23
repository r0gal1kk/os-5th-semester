#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    int num;
    char *str;
} my_struct;

void *mythread(void *arg) {
    my_struct *data = (my_struct *)arg;
    printf("mythread: num = %d, str = %s\n", data->num, data->str);
    return NULL;
}

int main() {
    pthread_t tid;
    int err;

    my_struct data = {42, "hello world"};

    err = pthread_create(&tid, NULL, mythread, &data);
    if (err) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        return -1;
    }

    err = pthread_join(tid, NULL);
    if (err) {
        printf("main: pthread_join() failed: %s\n", strerror(err));
        return -1;
    }

    return 0;
}