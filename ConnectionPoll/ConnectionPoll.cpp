#include "ConnectionPoll.h"
#include <cstring>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <thread>

using namespace Json;
using namespace std;

ConnectionPoll::ConnectionPoll() {
    //加载json文件
    if (!parseJsonFile()) {
        return;
    }
    for (int i = 0; i < m_minSize; i++) {
        addConection();
    }
    thread prodecer(&ConnectionPoll::produceConnection, this);
    thread recycler(&ConnectionPoll::recycleConnection, this);
    prodecer.detach();
    recycler.detach();
}

ConnectionPoll::~ConnectionPoll() {
    while (m_connectionQ.size()) {
        MysqlConn*conn = m_connectionQ.front();
        m_connectionQ.pop();
        delete conn;
    }
}

shared_ptr<MysqlConn> ConnectionPoll::getConnection() {
    unique_lock<mutex> locker(m_mutexQ);
    while (m_connectionQ.empty()) {
        if(cv_status::timeout == (m_cond.wait_for(locker, chrono::milliseconds(m_timeout)))) {
            if (m_connectionQ.empty()) {
                continue;
            }
        }
    }
    shared_ptr<MysqlConn> connptr(m_connectionQ.front(), [this](MysqlConn* conn) {
        lock_guard<mutex> locker(m_mutexQ);
        conn->refreshAliveTime();
        m_connectionQ.push(conn);
    });   //归还MysqlConn，操作析构函数
    m_connectionQ.pop();
    m_cond.notify_all();
    return connptr;
}

void ConnectionPoll::addConection() {
    if (m_connectionQ.size() >= m_maxSize) return;
    MysqlConn* conn = new MysqlConn();
    conn->connect(m_user, m_passwd, m_dbName, m_ip, m_port);
    conn->refreshAliveTime();
    m_connectionQ.push(conn);
}

ConnectionPoll *ConnectionPoll::getConnectionPoll() {
    static ConnectionPoll poll;  //单例模式，只有一个实例，只会创建一次
    return &poll;
}

bool ConnectionPoll::parseJsonFile() { 
    ifstream ifs("../dbconf.json");
    Reader rd;
    Value root;
    rd.parse(ifs, root);
    if (root.isObject()) {
        m_ip = root["ip"].asString();
        m_user = root["userName"].asString();
        m_passwd = root["password"].asString();
        m_dbName = root["dbName"].asString();
        m_port = root["port"].asInt();
        m_minSize = root["minSize"].asInt();
        m_maxSize = root["maxSize"].asInt();
        m_timeout = root["timeout"].asInt();
        m_maxIDleTime = root["maxIDleTime"].asInt();
        return true;
    }
    return false;
}

void ConnectionPoll::produceConnection() {
    while (true) {
        unique_lock<mutex> lock(m_mutexQ);
        while(m_connectionQ.size() >= m_minSize) {  //条件判断最好使用while循环而不是if判断
            m_cond.wait(lock);
        }
        addConection();
        m_cond.notify_all();
    }
    
}

void ConnectionPoll::recycleConnection() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(500));
        lock_guard<mutex> locker(m_mutexQ);
        while(m_connectionQ.size() > m_minSize) {  //条件判断最好使用while循环而不是if判断
            MysqlConn* conn = m_connectionQ.front();
            if (conn->getAliveTime() >= m_maxIDleTime) {
                m_connectionQ.pop();
                delete conn;
            } else {
                break;
            }
        } 
    }
}
