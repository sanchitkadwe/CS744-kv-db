#include "cache.h"
#include<iostream>
using namespace std;

LRUCache::LRUCache(size_t capacity) : cap(capacity) {}

void LRUCache::put(const string &key, const string &value) {
    lock_guard<mutex> lock(m);
    auto it = mp.find(key);
    if (it != mp.end()) {
        items.erase(it->second);
        mp.erase(it);
        cout << "[CACHE] Updated key " << key << endl;

    } else if (items.size() >= cap) {
        auto last = items.back().first;
        mp.erase(last);
        items.pop_back();
        cout << "[CACHE] Evicted key " << last << " (capacity full)" << endl;
    }
    items.emplace_front(key, value);
    mp[key] = items.begin();
    cout << "[CACHE] Inserted key " << key << " (size=" << items.size() << ")" << endl;

}

optional<string> LRUCache::get(const string &key) {
    lock_guard<mutex> lock(m);
    auto it = mp.find(key);
    if (it == mp.end()) {
        cout << "[CACHE] MISS for key " << key << endl;
        return {};
    }
    auto nodeIt = it->second;
    pair<string,string> kv = *nodeIt;
    items.erase(nodeIt);
    items.emplace_front(kv);
    mp[key] = items.begin();
    cout << "[CACHE] HIT for key " << key << endl;

    return items.begin()->second;
}

void LRUCache::erase(const string &key) {
    lock_guard<mutex> lock(m);
    auto it = mp.find(key);
    if (it == mp.end()) return;
    items.erase(it->second);
    mp.erase(it);
    cout << "[CACHE] Deleted key " << key << endl;

}

size_t LRUCache::size() {
    lock_guard<mutex> lock(m);
    return mp.size();
}
