#pragma once
#include <mysql/mysql.h>
#include <string>
using namespace std;

class SimpleDB {
public:
    SimpleDB(const string &host, const string &user, const string &pass, const string &db, unsigned int port=3306);
    ~SimpleDB();

    bool put_kv(int k, const string &v);   // key is int
    bool get_kv(int k, string &out_v);
    bool del_kv(int k);
    bool is_connected();

private:
    MYSQL *conn;
    string escape(MYSQL *c, const string &s);
};
