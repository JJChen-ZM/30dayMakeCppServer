#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "Socket.h"
#include "util.h"
#include "InetAddress.h"
#include <string.h>

Socket::Socket() : fd(-1){
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket create error");
}
Socket::Socket(int _fd) : fd(_fd){
    errif(fd == -1, "socket create error");
}

Socket::~Socket(){
    if(fd != -1){
        close(fd);
        fd = -1;
    }
}

void Socket::bind(InetAddress *_addr){
    // ::bind：这是调用系统的 bind 函数，用于将套接字绑定到指定的地址。:: 表示调用全局作用域的 bind 函数，以避免与类成员函数重名冲突。
    struct sockaddr_in addr = _addr->getAddr();
    socklen_t addr_len = _addr->getAddrLen();
    errif(::bind(fd, (sockaddr*)&addr, addr_len) == -1, "socket bind error");
    _addr->setInetAddr(addr, addr_len);
}

void Socket::listen(){
    errif(::listen(fd, SOMAXCONN) == -1, "socket listen error");
}

void Socket::setnonblocking(){
    int flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

int Socket::accept(InetAddress *_addr){
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    bzero(&addr, sizeof(addr));
    int clnt_sockfd = ::accept(fd, (sockaddr*)&addr, &addr_len);
    errif(clnt_sockfd == -1, "socket accept error");
    _addr->setInetAddr(addr, addr_len);
    return clnt_sockfd;
}

void Socket::connect(InetAddress *_addr){
    struct sockaddr_in addr = _addr->getAddr();
    socklen_t addr_len = _addr->getAddrLen();
    errif(::connect(fd, (sockaddr*)&addr, addr_len) == -1, "socket connect error");
}

int Socket::getFd(){
    return fd;
}