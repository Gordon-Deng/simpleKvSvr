#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>

class Epoll
{
public:
    int epollfd;

    Epoll() { epollfd = -1; }

    Epoll(int fd) { epollfd = fd; }

    ~Epoll() {}

    // 创建epoll的句柄
    int create()
    {
        epollfd = epoll_create(1);
        if (epollfd == -1)
            return -1;
        return 0;
    }

    int addEvent(int fd, int state)
    {
        epoll_event event;
        event.events = state;
        event.data.fd = fd;
        // epoll的事件注册函数
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    }

    int deleteEvent(int fd, int state)
    {
        epoll_event event;
        event.events = state;
        event.data.fd = fd;
        epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &event);
    }

    int modifyEvent(int fd, int state)
    {
        epoll_event event;
        event.events = state;
        event.data.fd = fd;
        epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
    }
};

#endif
