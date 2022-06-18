#include "threadpool.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <pthread.h>
#include <map>

using namespace std;

void *work(void *arg) {
    cout << 1 << endl;
    cout << "pthread_exit" << endl;
    pthread_exit(NULL);
    
}

int main() {
    threadpool *pool = threadpool::getThreadpool();
    threadpool_task_t *task = new threadpool_task_t();
    task->arg = NULL;
    task->function = work;
    pool->add_work(work, NULL);
    return 0;
}