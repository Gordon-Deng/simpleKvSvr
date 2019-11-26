#ifndef BASE_TCPSOCKET_H
#define BASE_TCPSOCKET_H
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "bug.h"
#include "error.h"

class BaseTcpSocket
{
public:
    int sockfd;

    BaseTcpSocket()
    {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            ERROR();
    }

    virtual ~BaseTcpSocket() { close(sockfd); }

    int Bind(sockaddr_in* addr)
    {
        int ret = bind(this->sockfd, (struct sockaddr*)addr, sizeof(struct sockaddr));
        return ret;
    }

    static int sendn(int sockfd, const char* buf, int nbytes, int flags)
    {
        int bytes = nbytes;
        const char* buffer = buf;
        int sent;
        while (bytes)
        {
            sent = send(sockfd, buf, bytes, flags);
            if (sent >= 0)
            {
                bytes -= sent;
                buffer += sent;
                continue;
            }
            else if (sent < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
            {
                continue;
            }
            else
            {
                ERROR(strerror(errno));
                break;
            }

            return nbytes - bytes;
        }
    }

    static int readn(int sockfd, char* buf, int nbytes, int flags, bool* isEOF)
    {
        int bytes = nbytes;
        int n;
        while (bytes)
        {
            n = recv(sockfd, buf, bytes, flags);
            if (n > 0)
            {
                bytes -= n;
                buf += n;
                continue;
            }
            if (n == 0)
            {
                if (isEOF != NULL)
                    *isEOF = true;
                break;
            }
            if (n < 0 && errno == EINTR)
            {
                continue;
            }
            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
                break;
            ERROR(strerror(errno));
            break;
        }
        return nbytes - bytes;
    }

    static int readline(int sockfd, char* buf, int nbytes)
    {
        bool isEnd;
        int n = recv(sockfd, buf, nbytes, MSG_PEEK);
        if (n == 0)
            return 0;
        char* position = buf;
        while (position < buf + n && *position != '\n')
            position++;
        if (position == buf + n)
            return -1;
        return recv(sockfd, buf, position - buf + 1, 0);
    }
};

#endif
