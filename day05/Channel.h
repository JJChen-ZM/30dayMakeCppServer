#pragma once
#include <sys/epoll.h>

class Epoll;
class Channel
{
private:
    Epoll *ep;
    int fd;
    uint32_t events;  // events表示希望监听这个文件描述符的哪些事件，因为不同事件的处理方式不一样
    uint32_t revents;  // revents表示在epoll返回该Channel时文件描述符正在发生的事件。
    bool inEpoll;
public:
    Channel(Epoll *_ep, int _fd);
    ~Channel();

    void enableReading();

    int getFd();
    uint32_t getEvents();
    uint32_t getRevents();
    bool getInEpoll();
    void setInEpoll();

    void setRevents(uint32_t);
};