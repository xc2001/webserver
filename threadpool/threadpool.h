#ifndef THREADPOOL_H
#define THREADPOOL_H

#pragma once
#include <iostream>
#include <algorithm>
#include <list>
#include "../sqlconnectionPoll/sqlconnectionPoll.h"
#include <cstdio>
#include <exception>
#include <pthread.h>

using namespace std;

typedef struct {
    void *(*function)(void *);          /* 函数指针，回调函数 */
    void *arg;                          /* 上面函数的参数 */
} threadpool_task_t;                    /* 各子线程任务结构体 */

class threadpool {
public:
    static threadpool* getThreadpool(int min_thr_num, int max_thr_num, int queue_max_size);
    int add_work(void*(*function)(void *arg), void *arg);
    threadpool(int min_thr_num, int max_thr_num, int queue_max_size);
private:    
    ~threadpool();
    
    threadpool(const threadpool& obj) = delete;
    threadpool& operator=(const threadpool& obj) = delete;
    void *threadpoolThread(void *pool);
    int threadpoolDestroy();
    static void *adjustThread(void *arg);
    static void *work(void *arg);
    
    sqlconnectionPoll* m_connectionPoll;
    mutex lockself;               /* 用于锁住本结构体 */    
    mutex thread_counter;     /* 记录忙状态线程个数de琐 -- busy_thr_num */

    condition_variable queue_not_full;      /* 当任务队列满时，添加任务的线程阻塞，等待此条件变量 */
    condition_variable queue_not_empty;     /* 任务队列里不为空时，通知等待任务的线程 */

    pthread_t *threads;                 /* 存放线程池中每个线程的tid。数组 */
    pthread_t adjust_tid;               /* 存管理线程tid */
    list<threadpool_task_t *> task_queue;      /* 任务队列(数组首地址) */

    int min_thr_num;                    /* 线程池最小线程数 */
    int max_thr_num;                    /* 线程池最大线程数 */
    int live_thr_num;                   /* 当前存活线程个数 */
    int busy_thr_num;                   /* 忙状态线程个数 */
    int wait_exit_thr_num;              /* 要销毁的线程个数 */
    int queue_max_size;                 /* task_queue队列可容纳任务数上限 */

    int shutdown;                       /* 标志位，线程池使用状态，true或false */
};

#endif