#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include "ThreadPool.h"

EventLoop::EventLoop() : ep(nullptr), threadPool(nullptr), quit(false){
    ep = new Epoll();
    threadPool = new ThreadPool();
}

EventLoop::~EventLoop(){
    delete ep;
}

void EventLoop::loop(){
    while(!quit){
        std::vector<Channel*> activeChannels = ep->poll();
        for(auto it = activeChannels.begin(); it!=activeChannels.end(); it++){
            (*it)->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel* chn){
    ep->updateChannel(chn);
}

void EventLoop::addThread(std::function<void()> func){
    threadPool->add(func);
}
