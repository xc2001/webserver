// #include "threadpool.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <pthread.h>

using namespace std;

void *work(void *arg) {
    cout << 1 << endl;
    pthread_exit(NULL);
    cout << "pthread_exit" << endl;
}

int main() {
    pthread_t pid;
    pthread_create(&pid, NULL, work, NULL);
    pthread_detach(pid);
    sleep(2);
    return 0;
}