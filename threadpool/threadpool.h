#ifndef THREADPOOL_H
#define THREADPOOL_H

#pragma once
#include <iostream>
#include <algorithm>
#include <queue>
#include <sqlconnectionPoll/sqlconnectionPoll.h>

using namespace std;

template <typename T>
class threadpool {
public:
    static threadpool* getThreadpool(int thread_number = 8, int max_request = 10000);
    ~threadpool();

private:
    threadpool();
    threadpool(const threadpool& obj);
    threadpool& operator=(const threadpool& obj) = delete;

private:
    sqlconnectionPoll* m_connectionPoll;
    int m_thread_number; // /线程池中的线程数
    int m_max_request; // 请求队列中允许的最大请求数
    pthread_t *m_threads; // 描述线程池的数组，其大小为m_thread_number
    list<T *> m_workqueue; // 用(底层是双向链表)做请求队列
    mutex m_queuelocker; // 保护请求队列的互斥锁
    condition_variable m_queuestat; // 是否有任务需要处理信号量
    bool m_stop; // 是否结束线程
};

#endif