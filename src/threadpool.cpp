#include "threadpool.h"
using namespace std;

ThreadPool::ThreadPool(size_t n){
    for(size_t i=0;i<n;i++)workers.emplace_back(&ThreadPool::loop,this);
}
void ThreadPool::loop(){
    while(true){
        function<void()> task;
        {
            unique_lock<mutex> lk(m);
            cv.wait(lk,[this]{return stop||!tasks.empty();});
            if(stop&&tasks.empty())return;
            task=move(tasks.front());
            tasks.pop();
        }
        task();
    }
}
void ThreadPool::enqueue(function<void()> f){
    {
        unique_lock<mutex> lk(m);
        tasks.push(move(f));
    }
    cv.notify_one();
}
void ThreadPool::shutdown(){
    {
        unique_lock<mutex> lk(m);
        stop=true;
    }
    cv.notify_all();
    for(auto &t:workers)if(t.joinable())t.join();
}
ThreadPool::~ThreadPool(){shutdown();}
