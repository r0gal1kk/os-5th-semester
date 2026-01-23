#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_THREADS 5

int global_var = 42;

void *mythread(void __attribute__((unused)) *arg) {
    pthread_t self_tid = pthread_self();  // POSIX tid
    pid_t tid_kernel = gettid();  // Kernel tid

    // Локальные переменные
    int local_var = 10;
    static int local_static_var = 20;

    printf("mythread [%d %d %d]: Hello! POSIX tid: %lu, Kernel tid: %d\n",
           getpid(), getppid(), tid_kernel, (unsigned long)self_tid, tid_kernel);

    local_var += tid_kernel % 10;
    global_var += tid_kernel % 10;
    local_static_var += tid_kernel % 10;

    printf("mythread: After change - Local: %d, Global: %d, Static: %d\n", local_var, global_var, local_static_var);

    return NULL;
}

int main() {
    pthread_t tid[NUM_THREADS];
    int err;
    int i;

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    for (i = 0; i < NUM_THREADS; i++) {
        err = pthread_create(&tid[i], NULL, mythread, NULL);
        
        if (err) {
            printf("main: pthread_create() failed for thread %d: %s\n", i, strerror(err));
            for (int j = 0; j < i; j++) {
                pthread_join(tid[j], NULL);
            }
            return -1;
        }
    }

    //sleep(300);

    for (int i = 0; i < NUM_THREADS; i++) {
        err = pthread_join(tid[i], NULL);
        if (err) {
            printf("main: pthread_join() failed for thread %d: %s\n", i, strerror(err));
            return -1;
        }
    }
    printf("main: Final global: %d\n", global_var);

    return 0;
}