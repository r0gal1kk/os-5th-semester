#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *thread_func(void __attribute__((unused)) *arg) {
    pthread_detach(pthread_self());
    unsigned long long counter = 0;
    while (1) {
        counter++;
        //if (counter % 100000000 == 0) printf("counter: %llu\n", counter);
    }
    return NULL;
}

int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, thread_func, NULL);
    sleep(2);
    printf("main: trying to cancel...\n");
    pthread_cancel(tid);
    //int *res;
    //int r = pthread_join(tid, (void**)&res);
    /*if (r == 0 && res == PTHREAD_CANCELED)
        printf("cancelled!\n");
    else
        printf("join returned %d, res = %p\n", r, res);*/
    //return 0;
    pthread_exit(NULL);
}