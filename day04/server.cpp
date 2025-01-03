#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/epoll.h>
#include <fcntl.h>
#include "util.h"
#include "Socket.h"
#include "Epoll.h"
#include "InetAddress.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void handleReadEvent(int);

int main(){
    Socket *serv_socket = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    serv_socket->bind(serv_addr);
    serv_socket->listen();

    Epoll *ep = new Epoll();
    serv_socket->setnonblocking();
    ep->addFd(serv_socket->getFd(), EPOLLIN | EPOLLET);
 
    while (true){
        std::vector<epoll_event> events = ep->poll();
        // int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        // errif(nfds == -1, "epoll wait error");
        for(int i=0; i<events.size(); i++){
            if (events[i].data.fd == serv_socket->getFd()) //新客户端连接
            {
                InetAddress *clnt_addr = new InetAddress(); //会发生内存泄露！没有delete
                int clnt_sockfd = serv_socket->accept(clnt_addr);
                Socket *clnt_socket = new Socket(clnt_sockfd); //会发生内存泄露！没有delete

                printf("new client fd %d! IP: %s Port: %d\n", clnt_socket->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
                
                clnt_socket->setnonblocking();
                ep->addFd(clnt_socket->getFd(), EPOLLIN | EPOLLET);
            } else if(events[i].events & EPOLLIN){  // 可读事件
                handleReadEvent(events[i].data.fd);
            } else{
                printf("something else happened\n");
            }
        }
    }    
    
    delete serv_socket;
    delete serv_addr;
    return 0;
}


void handleReadEvent(int sockfd){
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
            close(sockfd); //关闭socket会自动将文件描述符从epoll树上移除
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