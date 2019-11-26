#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include "basetcpsocket.h"

class SocketClient : public BaseTcpSocket
{
public:
    int Connect(sockaddr_in* addr) { return connect(sockfd, (sockaddr*)addr, sizeof(sockaddr_in)); }
};
#endif
