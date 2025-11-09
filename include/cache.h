#include <string>
#include <list>
#include <unordered_map>
#include <optional>
#include <mutex>
#pragma once
using namespace std;

class LRUCache {
public:
    LRUCache(size_t capacity = 100);
    void put(const string &key, const string &value);
    optional<string> get(const string &key);
    void erase(const string &key);
    size_t size();

private:
    size_t cap;
    list<pair<string,string>> items;
    unordered_map<string, list<pair<string,string>>::iterator> mp;
    mutable mutex m; // protect cache
};
