#include "db.h"
#include <iostream>
#include <sstream>
using namespace std;

SimpleDB::SimpleDB(const string &host, const string &user, const string &pass, const string &db, unsigned int port) {
    conn = mysql_init(NULL);
    if (!conn) { cerr << "mysql_init failed\n"; return; }
    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(), db.c_str(), port, NULL, 0)) {
        cerr << "mysql_real_connect error: " << mysql_error(conn) << endl;
        mysql_close(conn); conn = nullptr;
    }
}
SimpleDB::~SimpleDB() { if (conn) mysql_close(conn); }
bool SimpleDB::is_connected() { return conn != nullptr; }

string SimpleDB::escape(MYSQL *c, const string &s) {
    char *buf = new char[s.size()*2+1];
    unsigned long len = mysql_real_escape_string(c, buf, s.c_str(), s.size());
    string out(buf, len);
    delete[] buf;
    return out;
}

bool SimpleDB::put_kv(int k, const string &v) {
    if (!conn) return false;
    string ev = escape(conn, v);
    stringstream ss;
    ss << "INSERT INTO kv_store (k,v) VALUES (" << k << ", '" << ev
       << "') ON DUPLICATE KEY UPDATE v=VALUES(v)";
    return mysql_query(conn, ss.str().c_str()) == 0;
}

bool SimpleDB::get_kv(int k, string &out_v) {
    if (!conn) return false;
    stringstream ss;
    ss << "SELECT v FROM kv_store WHERE k=" << k << " LIMIT 1";
    if (mysql_query(conn, ss.str().c_str()) != 0) return false;
    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) return false;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) { mysql_free_result(res); return false; }
    out_v = row[0] ? string(row[0]) : "";
    mysql_free_result(res);
    return true;
}

bool SimpleDB::del_kv(int k) {
    if (!conn) return false;
    stringstream ss;
    ss << "DELETE FROM kv_store WHERE k=" << k;
    return mysql_query(conn, ss.str().c_str()) == 0;
}
