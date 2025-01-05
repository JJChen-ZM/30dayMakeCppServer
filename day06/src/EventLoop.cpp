#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"

EventLoop::EventLoop() : ep(nullptr), quit(false){
    ep = new Epoll();
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