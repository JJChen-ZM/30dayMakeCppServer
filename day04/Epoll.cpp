#include <string.h>
#include <unistd.h>
#include "Epoll.h"
#include "util.h"

#define MAX_EVENTS 1024
Epoll::Epoll():epfd(-1), events(nullptr)
{
    epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");
    events = new epoll_event[MAX_EVENTS]; // 这行代码的主要功能是在堆内存中分配足够的空间来存储 MAX_EVENTS 个 epoll_event 结构体，并将指向这块内存的指针赋值给 events 变量。
    bzero(events, sizeof(*events) * MAX_EVENTS);
}

Epoll::~Epoll()
{
    if(epfd != -1){
        close(epfd);
        epfd = -1;
    }
    delete[] events;
}


void Epoll::addFd(int fd, uint32_t op)
{
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.fd = fd;
    ev.events = op;
    errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
}

std::vector<epoll_event> Epoll::poll(int timeout)
{
    std::vector<epoll_event> activeEvents;
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    errif(nfds == -1, "epoll wait error");
    for(int i = 0; i < nfds; ++i){
        activeEvents.push_back(events[i]);
    }
    return activeEvents;
}