#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void handler(int __attribute__((unused)) sig) {

}

void *foreign_code(void __attribute__((unused)) *arg) {
    unsigned int count = 0;
    while (1) {
        count++;
    }
    return NULL;
}

int main() {
    pthread_t tid;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    struct sigaction sa = {0};
    sa.sa_handler = handler;
    sigaction(SIGUSR1, &sa, NULL);

    pthread_create(&tid, NULL, foreign_code, NULL);

    sleep(1);

    printf("main: Cancelling thread...\n");
    pthread_cancel(tid);

    pthread_kill(tid, SIGUSR1);

    pthread_join(tid, NULL);

    printf("Thread cancelled\n");
}
