#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Acceptor.h"

Server::Server(EventLoop *_loop) : loop(_loop){
    // Socket *serv_socket = new Socket();
    // InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    // serv_socket->bind(serv_addr);
    // serv_socket->listen();
    // serv_socket->setnonblocking();
    
    // Channel *servChannel = new Channel(loop, serv_socket->getFd());

    // std::function<void()> cb = std::bind(&Server::newConnection, this, serv_socket);

    // servChannel->setCallback(cb);
    // servChannel->enableReading();
    acceptor = new Acceptor(loop);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1)
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server(){
    delete acceptor;
}

void Server::handleReadEvent(int sockfd){
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

void Server::newConnection(Socket *serv_socket){
    InetAddress *clnt_addr = new InetAddress(); //会发生内存泄露！没有delete
    int clnt_sockfd = serv_socket->accept(clnt_addr);
    Socket *clnt_socket = new Socket(clnt_sockfd); //会发生内存泄露！没有delete

    printf("new client fd %d! IP: %s Port: %d\n", clnt_socket->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
    
    clnt_socket->setnonblocking();

    Channel *clntChannel = new Channel(loop, clnt_socket->getFd());

    std::function<void()> cb = std::bind(&Server::handleReadEvent, this, clnt_socket->getFd());
    
    clntChannel->setCallback(cb);
    clntChannel->enableReading();
}