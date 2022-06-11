#include "MysqlConn.h"
#include "ConnectionPoll.h"
#include <iostream>
#include <unistd.h>
#include <thread>

using namespace std;

int query() {
    MysqlConn *conn = new MysqlConn();
    conn->connect("root", "", "", "localhost", 3306);
    string sql = "INSERT INTO `collegeid` (`collegeid`, `collegename`, `whichcampus`) VALUES(5, '计算机学院', '厦门');";
    bool flag = conn->updata(sql);
    sql = "select * from collegeid";
    conn->query(sql);
    return 0;
} 

void op1(int begin, int end) {
    for (int i = begin; i < end; i++) {
        MysqlConn *conn = new MysqlConn();
        conn->connect("root", "", "", "localhost", 3306);
        string sql = "INSERT INTO `collegeid` (`collegeid`, `collegename`, `whichcampus`) VALUES(5, '计算机学院', '厦门');";
        conn->updata(sql);
        delete conn;
    }
}

void op2(ConnectionPoll* pool, int begin, int end) {
    for (int i = begin; i < end; i++) {
        shared_ptr<MysqlConn> conn = pool->getConnection();
        string sql = "INSERT INTO `collegeid` (`collegeid`, `collegename`, `whichcampus`) VALUES(5, '计算机学院', '厦门');";
        conn->updata(sql);
    }
}

void test1() {
#if 0
    steady_clock::time_point begin = steady_clock::now();
    op1(0, 5000);
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << "非连接池 单线程 用时" << length.count() << "纳秒," << length.count()/ 1000000 << "毫秒" << endl;
#else 
    ConnectionPoll *pool = ConnectionPoll::getConnectionPoll();
    steady_clock::time_point begin = steady_clock::now();
    op2(pool, 0, 5000);
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << "连接池 单线程 用时" << length.count() << "纳秒," << length.count()/ 1000000 << "毫秒" << endl;
#endif
}

void test2() {
#if 0
    MysqlConn* conn = new MysqlConn();
    conn->connect("root", "Xuchi3212016614@", "studyMysql", "localhost", 3306);
    steady_clock::time_point begin = steady_clock::now();
    thread t1(op1, 0, 1000);
    thread t2(op1, 1000, 2000);
    thread t3(op1, 2000, 3000);
    thread t4(op1, 3000, 4000);
    thread t5(op1, 4000, 5000);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << "非连接池 多线程 用时" << length.count() << "纳秒," << length.count()/ 1000000 << "毫秒" << endl;
#else 
    ConnectionPoll *pool = ConnectionPoll::getConnectionPoll();
    steady_clock::time_point begin = steady_clock::now();
    thread t1(op2, pool, 0, 1000);
    thread t2(op2, pool, 1000, 2000);
    thread t3(op2, pool, 2000, 3000);
    thread t4(op2, pool, 3000, 4000);
    thread t5(op2, pool, 4000, 5000);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << "连接池 多线程 用时" << length.count() << "纳秒," << length.count()/ 1000000 << "毫秒" << endl;
#endif
}

int main() {
    // test1();
    test2();
}