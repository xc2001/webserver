#include "threadpool.h"

using namespace std;

sqlconnectionPoll* m_connectionPoll;  /* 数据库连接池*/
pthread_mutex_t lockself;               /* 用于锁住本结构体 */    
pthread_mutex_t thread_counter;     /* 记录忙状态线程个数de琐 -- busy_thr_num */

pthread_cond_t queue_not_full;      /* 当任务队列满时，添加任务的线程阻塞，等待此条件变量 */
pthread_cond_t queue_not_empty;     /* 任务队列里不为空时，通知等待任务的线程 */

pthread_t *threads;                 /* 存放线程池中每个线程的tid。数组 */
pthread_t adjust_tid;               /* 存管理线程tid */
list<threadpool_task_t *> task_queue;      /* 任务队列(数组首地址) */

int min_thr_num;                    /* 线程池最小线程数 */
int max_thr_num;                    /* 线程池最大线程数 */
int live_thr_num;                   /* 当前存活线程个数 */
int busy_thr_num;                   /* 忙状态线程个数 */
int wait_exit_thr_num;              /* 要销毁的线程个数 */
int queue_max_size;                 /* task_queue队列可容纳任务数上限 */

bool shutdown;                       /* 标志位，线程池使用状态，true或false */
threadpool* threadpool::getThreadpool(int min_thr_num, int max_thr_num, int queue_max_size) {
    threadpool *pool = new threadpool(min_thr_num, max_thr_num, queue_max_size);
    return pool;
}

threadpool::threadpool(int min_thr_num, int max_thr_num, int queue_max_size):min_thr_num(min_thr_num), max_thr_num(max_thr_num), queue_max_size(queue_max_size) {
    m_connectionPoll = sqlconnectionPoll::getsqlconnectionPoll();
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
    pthread_detach(adjust_tid);
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
    task_queue.push_back(task);
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
                    pthread_exit(NULL);
                }
            }
        }
        
        if (pool->shutdown == true) {
            printf("thread 0x%x is exiting\n", pthread_self());
            pthread_exit(NULL);
        }
    }
}

void* threadpool::adjustThread(void *pool) {
    
}
