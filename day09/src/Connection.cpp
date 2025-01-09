#include <iostream>
#include <unistd.h>
#include <string.h>
#include "Connection.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"

#define READ_BUFFER 1024

Connection::Connection(EventLoop *_loop, Socket *_sock): loop(_loop), sock(_sock)
{
    channel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setCallback(cb);
    channel->enableReading();
}

Connection::~Connection()
{
    delete channel;
    delete sock;
}

void Connection::echo(int sockfd)
{
    char buf[READ_BUFFER];
    while (true){ // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0)
        {
            std::cout << "message from client fd " << sockfd << ": " << buf << std::endl;
            write(sockfd, buf, sizeof(buf));
        } else if(read_bytes == 0){ //EOF事件，客户端断开连接
            printf("EOF, client fd %d disconnected\n", sockfd);
            // close(sockfd); //关闭socket会自动将文件描述符从epoll树上移除
            deleteConnectionCallback(sock);
            break;
        } else if(read_bytes == -1 && errno == EINTR){ // 客户端正常中断，继续读取。EINTR 表示操作被信号中断。在这种情况下，读取操作并非真正的错误，只是被系统信号打断了。
            printf("continue reading");
            continue;
        } else if(read_bytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){  //非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            break;
        }
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> _cb)
{
    deleteConnectionCallback = _cb;
}