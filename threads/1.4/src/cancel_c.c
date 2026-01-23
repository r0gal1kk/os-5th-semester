#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void cleanup(void *arg) {
    free(arg);
    printf("cleanup: memory freed\n");
}

void *thread_func(void __attribute__((unused)) *arg) {
    char *str = malloc(20);
    strcpy(str, "hello world");

    pthread_cleanup_push(cleanup, str);

    while (1) {
        printf("%s\n", str);
        sleep(1);
    }

    pthread_cleanup_pop(1);
    return NULL;
}

int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, thread_func, NULL);
    sleep(3);
    printf("main: cancelling...\n");
    pthread_cancel(tid);
    pthread_join(tid, NULL);
    printf("thread cancelled, memory freed\n");
    return 0;
}