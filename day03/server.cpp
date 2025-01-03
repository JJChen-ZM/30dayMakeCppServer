#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "util.h"
#include <unistd.h>
#include <iostream>
#include <sys/epoll.h>
#include <fcntl.h>

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void setnonblocking(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

int main(){
    int sockfd =  socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8888);

    errif(bind(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1, "socket bind error");

    errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

    int epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");

    struct epoll_event events[MAX_EVENTS], ev;
    bzero(&events, sizeof(events));

    bzero(&ev, sizeof(ev));
    ev.data.fd = sockfd;
    ev.events = EPOLLIN | EPOLLET;
    setnonblocking(sockfd);
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    while (true){
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        errif(nfds == -1, "epoll wait error");

        for (int i = 0; i < nfds; i++){
            if (events[i].data.fd == sockfd){   //新客户端连接
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                bzero(&client_addr, sizeof(client_addr));
                
                int clnt_sockfd = accept(sockfd, (sockaddr*)&client_addr, &client_addr_len);
                errif(clnt_sockfd == -1, "socket accept error");

                printf("new client fd %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                bzero(&ev, sizeof(ev));
                ev.data.fd = clnt_sockfd;
                ev.events = EPOLLIN | EPOLLET;
                setnonblocking(clnt_sockfd);
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &ev);
            } else if(events[i].events & EPOLLIN){  // 可读事件
                char buf[READ_BUFFER];
                while (true){ // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
                    bzero(&buf, sizeof(buf));
                    ssize_t read_bytes = read(events[i].data.fd, buf, sizeof(buf));
                    if (read_bytes > 0)
                    {
                        std::cout << "message from client fd " << events[i].data.fd << ": " << buf << std::endl;
                        write(events[i].data.fd, buf, sizeof(buf));
                    } else if(read_bytes == 0){ //EOF事件，客户端断开连接
                        printf("EOF, client fd %d disconnected\n", events[i].data.fd);
                        close(events[i].data.fd); //关闭socket会自动将文件描述符从epoll树上移除
                        break;
                    } else if(read_bytes == -1 && errno == EINTR){ // 客户端正常中断，继续读取。EINTR 表示操作被信号中断。在这种情况下，读取操作并非真正的错误，只是被系统信号打断了。
                        printf("continue reading");
                        continue;
                    } else if(read_bytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){  //非阻塞IO，这个条件表示数据全部读取完毕
                        printf("finish reading once, errno: %d\n", errno);
                        break;
                    }
                }
            } else{
                printf("something else happened\n");
            }
        }
    }    
    
    close(sockfd);

    return 0;
}