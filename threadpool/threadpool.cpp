#include "threadpool.h"

using namespace std;
int idx = 0;

threadpool* threadpool::getThreadpool(int min_thr_num, int max_thr_num, int queue_max_size) {
    threadpool *pool = new threadpool(min_thr_num, max_thr_num, queue_max_size);
    return pool;
}

threadpool::threadpool(int min_thr_num, int max_thr_num, int queue_max_size):min_thr_num(min_thr_num), max_thr_num(max_thr_num), queue_max_size(queue_max_size) {
    busy_thr_num = 0;
    live_thr_num = min_thr_num;
    wait_exit_thr_num = 0;
    shutdown = false;
    threads = new pthread_t[max_thr_num];
    if (threads == nullptr) 
        throw std::exception();
    for (int i = 0; i < min_thr_num; i++) {
        if (pthread_create(threads + i, NULL, work, this) != 0) {
            delete[] threads;
            throw std::exception();
        } 
        if (pthread_detach(threads[i]) != 0) {
            delete[] threads;
            throw std::exception();
        }
    }
    pthread_create(&adjust_tid, NULL, adjustThread, (void *)this);
}

threadpool::~threadpool() {
    while (task_queue.size()) {
        threadpool_task_t *task = task_queue.front();
        delete task;
        task_queue.pop();
    }
    shutdown = true;
    pthread_join(adjust_tid, NULL);
    while(live_thr_num) {
        queue_not_empty.notify_all();
    }
    cout << "live_thr_num:" << live_thr_num << endl;
    delete[] threads;
    
}

int threadpool::add_work(void*(*function)(void *arg), void *arg) {
    unique_lock<mutex> lock(lockself);
    while (task_queue.size() == max_thr_num && this->shutdown == false) {
        queue_not_full.wait(lock);
    }
    if (this->shutdown == true) {
        queue_not_empty.notify_all();
        return 0;
    }
    threadpool_task_t *task = new threadpool_task_t();
    task->function = function;
    task->arg = arg;
    task_queue.push(task);
    queue_not_empty.notify_one();
    return 0;
}

void* threadpool::work(void *arg) {
    threadpool *pool = (threadpool *)arg;
    while (true) {
        unique_lock<mutex> lock(pool->lockself);
        while (pool->task_queue.size() == 0 && pool->shutdown == false) {
            pool->queue_not_empty.wait(lock);
            if (pool->wait_exit_thr_num > 0) {
                pool->wait_exit_thr_num--;
                if (pool->live_thr_num > pool->min_thr_num) {
                    printf("thread 0x%x is exiting\n", pthread_self());
                    pool->live_thr_num--;
                    return nullptr;
                }
            }
        }
        if (pool->shutdown == true) {
            printf("thread 0x%x is exiting\n", pthread_self());
            pool->live_thr_num--;
            return nullptr;
        }

        threadpool_task_t *task = pool->task_queue.front();
        pool->task_queue.pop();
        pool->queue_not_full.notify_all();
        lock.unlock();
        printf("thread 0x%x start working\n", pthread_self());
        lock_guard<mutex> busy_locker(pool->thread_counter);
        pool->busy_thr_num++;
        (*(task->function))(task->arg);
        printf("thread 0x%x end working\n", pthread_self());
        pool->busy_thr_num--;
    }
    pthread_exit(NULL);
}

void* threadpool::adjustThread(void *arg) {
    // threadpool *pool = (threadpool *)arg;
    // while (!pool->shutdown) {
    //     lock_guard<mutex> locker(pool->lockself);
    //     lock_guard<mutex> busyLocker(pool->lockself);

    // }
}
