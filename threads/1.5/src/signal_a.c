#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

void sigint_handler(int sig) {
    printf("[THREAD 2] Caught SIGINT (%d)\n", sig);
}

void *thread1_block_all(void __attribute__((unused)) *arg) {
    sigset_t set;
    sigfillset(&set);

    pthread_sigmask(SIG_BLOCK, &set, NULL);
    printf("[THREAD 1] All signals blocked.\n");
    while (1) pause();
    return NULL;
}

void *thread2_sigint_handler(void __attribute__((unused)) *arg) {
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    signal(SIGINT, sigint_handler);
    printf("[THREAD 2] SIGINT handler installed.\n");

    while (1) pause();
    return NULL;
}

void *thread3_sigwait(void __attribute__((unused)) *arg) {
    sigset_t set;
    int sig;

    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);

    pthread_sigmask(SIG_BLOCK, &set, NULL);

    printf("[THREAD 3] Waiting for SIGQUIT with sigwait()...\n");

    sigwait(&set, &sig);
    printf("[THREAD 3] sigwait() got signal %d (SIGQUIT)\n", sig);

    return NULL;
}

int main() {
    printf("my pid:%d\n", getpid());
    pthread_t t1, t2, t3;

    sigset_t all;
    sigemptyset(&all);
    sigaddset(&all, SIGINT);
    sigaddset(&all, SIGQUIT);
    pthread_sigmask(SIG_BLOCK, &all, NULL);

    pthread_create(&t1, NULL, thread1_block_all, NULL);
    pthread_create(&t2, NULL, thread2_sigint_handler, NULL);
    pthread_create(&t3, NULL, thread3_sigwait, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    return 0;
}
