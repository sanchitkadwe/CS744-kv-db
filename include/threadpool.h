#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
using namespace std;

class ThreadPool {
public:
    ThreadPool(size_t n);
    ~ThreadPool();
    void enqueue(function<void()> f);
    void shutdown();
private:
    vector<thread> workers;
    queue<function<void()>> tasks;
    mutex m;
    condition_variable cv;
    bool stop=false;
    void loop();
};
