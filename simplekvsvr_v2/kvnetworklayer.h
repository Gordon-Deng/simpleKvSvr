#ifndef KV_NETWORKLAYER_H
#define KV_NETWORKLAYER_H

#include <sys/time.h>
#include <sys/socket.h>
#include "thread.h"
#include "queue.h"
#include "kvlogiclayer.h"
#include "task.h"
#include "basetcpsocket.h"
#include "serversocket.h"
#include "epoll.h"

class KVNetworkLayer
{
private:
public:
    KVLogicLayer* logicLayer;
    Queue<int>* fdQueue;
    Epoll* epoll;
    SocketServer* socket;
    int numOfEpollThr, numOfInputThr, numOfOutputThr;
    Thread **epollThrList, **inputThrList, **outputThrList;

    KVNetworkLayer(KVLogicLayer* logical, int queueSize, int epollNum, int inputNum, int outputNum)
    {
        this->logicLayer = logical;
        this->fdQueue = new Queue<int>(queueSize);
        this->epoll = new Epoll();
        this->epoll->create();
        this->socket = new SocketServer();
        this->numOfEpollThr = epollNum;
        this->numOfInputThr = inputNum;
        this->numOfOutputThr = outputNum;
        this->epollThrList = new Thread*[epollNum];
        this->inputThrList = new Thread*[inputNum];
        this->outputThrList = new Thread*[outputNum];
        for (int i = 0; i < epollNum; i++)
        {
            this->epollThrList[i] = new Thread(&epollThread, this);
        }
        for (int i = 0; i < inputNum; i++)
        {
            this->inputThrList[i] = new Thread(&inputThread, this);
        }
        for (int i = 0; i < outputNum; i++)
        {
            this->outputThrList[i] = new Thread(&outputThread, this);
        }
    }

    int run(int port, const char* ip)
    {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip);
        if (this->socket->Bind(&addr) < 0)
        {
            ERROR(strerror(errno));
        }
        if (this->socket->Listen() < 0)
        {
            ERROR(strerror(errno));
        }

        int fd;

        for (;;)
        {
            fd = this->socket->Accept(NULL, NULL);
            if (fd < 0)
            {
                if (errno == EINTR)
                    continue;
                ERROR(strerror(errno));
                continue;
            }
            this->epoll->addEvent(fd, EPOLLIN | EPOLLONESHOT);
        }
    }

    static void* epollThread(void* arg)
    {
        KVNetworkLayer* thisLayer = (KVNetworkLayer*)arg;
        epoll_event events[1024];
        for (;;)
        {
            int n = epoll_wait(thisLayer->epoll->epollfd, events, 1024, 10);
            if (n > 0)
            {
                for (int i = 0; i < n; i++)
                {
                    thisLayer->fdQueue->enqueue(events[i].data.fd);
                }
            }
            else if (n < 0)
            {
                ERROR(strerror(errno));
            }
        }
    }


    static void* inputThread(void* arg)
    {
        KVNetworkLayer* thisLayer = (KVNetworkLayer*)arg;
        Task* task;
        int fd;
        int n;
        for (;;)
        {
            fd = thisLayer->fdQueue->dequeue();
            task = new Task();
            n = BaseTcpSocket::readline(fd, task->buf, 4096);
            if (n < 0)
            {
                thisLayer->epoll->modifyEvent(fd, EPOLLIN | EPOLLONESHOT);
                delete task;
                continue;
            }
            if (n == 0)
            {
                shutdown(fd, SHUT_WR);
                delete task;
                continue;
            }
            if (task->buf[0] == '\n')
            {
                thisLayer->epoll->modifyEvent(fd, EPOLLIN | EPOLLONESHOT);
                delete task;
                continue;
            }
            task->fd = fd;
            task->n = n;
            if (task->buf[n - 2] == '\r')
            {
                task->buf[n - 2] = '\n';
                task->n = n - 1;
            }
            thisLayer->logicLayer->addTask(task);
            thisLayer->epoll->modifyEvent(fd, EPOLLIN | EPOLLONESHOT);
        }
    }


    static void* outputThread(void* arg)
    {
        KVNetworkLayer* thisLayer = (KVNetworkLayer*)arg;
        int oldtime, newtime;
        Task* task;
        for (;;)
        {
            task = thisLayer->logicLayer->getResult();
            BaseTcpSocket::sendn(task->fd, task->res, task->resLen, 0);
            if (task->CMD == QUIT)
            {
                shutdown(task->fd, SHUT_WR);
                thisLayer->epoll->deleteEvent(task->fd, 0);
            }
            if (!task->cached)
                delete task;
        }
    }
};

#endif
