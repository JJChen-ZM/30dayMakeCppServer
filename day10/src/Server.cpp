#include <iostream>
#include <unistd.h>
#include <string.h>
#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "Channel.h"
#include "Connection.h"

#define READ_BUFFER 1024

Server::Server(EventLoop *_loop) : loop(_loop), acceptor(nullptr)
{
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
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server(){
    delete acceptor;
}

// 注意：day07 newConnection的函数只针对 server_socket, 而day08 newConnection的函数针对的是所有的socket，因此不需要通过 server_socket来accept client，而是通过newConnection函数传参的方式来获取 client sock。
void Server::newConnection(Socket *sock){
    // InetAddress *clnt_addr = new InetAddress(); //会发生内存泄露！没有delete
    // int clnt_sockfd = serv_socket->accept(clnt_addr);
    // Socket *clnt_socket = new Socket(clnt_sockfd); //会发生内存泄露！没有delete

    // printf("new client fd %d! IP: %s Port: %d\n", clnt_socket->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
    
    // clnt_socket->setnonblocking();

    // Channel *clntChannel = new Channel(loop, clnt_socket->getFd());

    // std::function<void()> cb = std::bind(&Server::handleReadEvent, this, clnt_socket->getFd());
    
    // clntChannel->setCallback(cb);
    // clntChannel->enableReading();

    Connection *conn = new Connection(loop, sock);
    std::function<void(Socket*)> cb = std::bind(&Server::deleteConnnection, this, std::placeholders::_1);
    conn->setDeleteConnectionCallback(cb);
    connections[sock->getFd()] = conn; // map插入元素
}

void Server::deleteConnnection(Socket *sock)
{
    Connection *conn = connections[sock->getFd()];
    connections.erase(sock->getFd());
    delete conn;
}
