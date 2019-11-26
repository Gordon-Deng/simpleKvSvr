#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include "basetcpsocket.h"

#define MAX_LISTEN 20

class SocketServer : public BaseTcpSocket
{
public:
    int Listen() { return listen(sockfd, MAX_LISTEN); }

    int Accept(sockaddr_in* addr, socklen_t* len) { return accept(sockfd, (sockaddr*)addr, len); }
};

#endif
