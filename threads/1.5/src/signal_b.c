#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

void handler_A(int sig) {
    printf("[THREAD] Handler A caught %d\n", sig);
}

void handler_B(int sig) {
    printf("[THREAD] Handler B caught %d\n", sig);
}

void *threadA(void __attribute__((unused)) *arg) {
    printf("[THREAD A] Installing handler A\n");
    signal(SIGUSR1, handler_A);

    while (1) pause();
    return NULL;
}

void *threadB(void __attribute__((unused)) *arg) {
    printf("[THREAD B] Installing handler B\n");
    sleep(1);
    signal(SIGUSR1, handler_B);

    while (1) pause();
    return NULL;
}

int main() {
    pthread_t a, b;
    pthread_create(&a, NULL, threadA, NULL);
    pthread_create(&b, NULL, threadB, NULL);

    sleep(2);
    printf("[MAIN] Sending SIGUSR1...\n");
    raise(SIGUSR1);

    pthread_join(a, NULL);
    pthread_join(b, NULL);

    return 0;
}
