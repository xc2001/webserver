#ifndef sqlconnectionPoll_H
#define sqlconnectionPoll_H

#pragma once
#include <iostream>
#include <mysql.h>
#include "MysqlConn.h"
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

using namespace std; 

class sqlconnectionPoll {
public:
    static sqlconnectionPoll* getsqlconnectionPoll();
    sqlconnectionPoll(const sqlconnectionPoll& obj) = delete;
    sqlconnectionPoll& operator=(const sqlconnectionPoll& obj) = delete;
    shared_ptr<MysqlConn> getConnection();
    ~sqlconnectionPoll();
private:
    sqlconnectionPoll();
    bool parseJsonFile();
    void produceConnection();
    void recycleConnection();
    void addConection();
    queue<MysqlConn*> m_connectionQ;
    string m_ip;
    string m_user;
    string m_passwd;
    string m_dbName;
    unsigned short m_port;
    int m_minSize;
    int m_maxSize;
    int m_timeout;
    int m_maxIDleTime;
    mutex m_mutexQ;
    condition_variable m_cond; 
};

#endif
