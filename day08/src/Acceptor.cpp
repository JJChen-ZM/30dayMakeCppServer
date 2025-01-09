#include "Acceptor.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Server.h"

Acceptor::Acceptor(EventLoop *_loop) : loop(_loop)
{
    sock = new Socket();
    addr = new InetAddress("127.0.0.1", 8888);
    sock->bind(addr);
    sock->listen();
    sock->setnonblocking();
    
    acceptChannel = new Channel(loop, sock->getFd());

    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);

    acceptChannel->setCallback(cb);
    acceptChannel->enableReading();
}

Acceptor::~Acceptor()
{
    delete sock;
    delete addr;
    delete acceptChannel;
}

void Acceptor::acceptConnection()
{
    InetAddress *clnt_addr = new InetAddress(); //会发生内存泄露！没有delete
    int clnt_sockfd = sock->accept(clnt_addr);
    Socket *clnt_socket = new Socket(clnt_sockfd); //会发生内存泄露！没有delete

    printf("new client fd %d! IP: %s Port: %d\n", clnt_socket->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
    
    clnt_socket->setnonblocking();

    newConnectionCallback(clnt_socket);
    delete clnt_addr;
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> _cb)
{
    newConnectionCallback = _cb;
}
