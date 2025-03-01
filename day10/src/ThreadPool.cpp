#include "ThreadPool.h"


// std::vector<std::thread> threads;
// std::queue<std::function<void()>> tasks;
// std::mutex tasks_mtx;
// std::condition_variable cv;
// bool stop;

ThreadPool::ThreadPool(int size = 10) : stop(false){
    for(int i=0; i<size; i++){
        threads.emplace_back(std::thread([this](){
            while(true){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(tasks_mtx);
                    cv.wait(lock, [this](){
                        return stop || !tasks.empty();
                    });
                    if(stop && tasks.empty()) return;
                    task = tasks.front();
                    tasks.pop();
                }
                task();
            }
        }));
    }
}

ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);
        stop = true;
    }
    cv.notify_all();
    for(std::thread &th : threads){
        if(th.joinable()){
            th.join();
        }
    }
}

void ThreadPool::add(std::function<void()> func){
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);
        if(stop)
            throw std::runtime_error("ThreadPool already stop, can't add task any more");
        tasks.emplace_back(func);
    }
    cv.notify_one();
}

