#include <string.h>
#include "InetAddress.h"

InetAddress::InetAddress(){
    bzero(&addr, sizeof(addr));
    addr_len = sizeof(addr);
}

InetAddress::InetAddress(const char* ip, uint16_t port){
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    addr_len = sizeof(addr);
}

InetAddress::~InetAddress(){
}


void InetAddress::setInetAddr(sockaddr_in _addr, socklen_t _addr_len){
    addr = _addr;
    addr_len = _addr_len;
}
sockaddr_in InetAddress::getAddr(){
    return addr;
}
socklen_t InetAddress::getAddrLen(){
    return addr_len;
}