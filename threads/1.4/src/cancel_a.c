#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *thread_func(void __attribute__((unused)) *arg) {
    while (1) {
        printf("thread func working...\n");
        usleep(100000);
    }
    return NULL;
}

int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, thread_func, NULL);
    sleep(1);
    printf("main: cancelling thread\n");
    pthread_cancel(tid);
    pthread_join(tid, NULL);
    printf("thread cancelled and joined\n");
    return 0;
}
