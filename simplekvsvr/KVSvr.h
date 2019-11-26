#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ThreadPool.h"
#include "Worker.h"
#include "workqueue.h"
#include <iostream>
using namespace std;

class KVSvr
{
    int sock_;
    int status_ = 0;
    sockaddr_in local_;
    workqueue<pair<string, int>>* from_queue_;

public:
    KVSvr(int port, char* ip, workqueue<pair<string, int>>* from_queue)
    {
        from_queue_ = from_queue;
        sock_ = socket(AF_INET, SOCK_STREAM, 0);
        int opt = SO_REUSEADDR;
        setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (sock_ < 0)
        {
            status_ = -1;
            return;
        }
        local_.sin_family = AF_INET;
        local_.sin_port = htons(port);
        local_.sin_addr.s_addr = inet_addr(ip);
    }
    int Run()
    {
        if (status_ < 0)
            return -1;
        //workqueue<int> fdqueue;
        //FdWorker fdworker(&fdqueue);
        //ThreadPool threadpool(10,&fdworker);
        //threadpool.Start();
        socklen_t len = sizeof(local_);
        if (bind(sock_, (sockaddr*)&local_, len) < 0)
        {
            puts("failed to bind");
            printf("errno	 %d\n", errno);
            return -2;
        }
        if (listen(sock_, 5) < 0)
        {
            puts("failed to listen");
            return -3;
        }
        sockaddr_in remote;
        len = sizeof(sockaddr_in);

        fd_set rfds;
        struct timeval tv;
        int retval, maxfd;
        while (1)
        {
            int fd = accept(sock_, (sockaddr*)&remote, &len);
            if (fd < 0)
            {
                puts("failed to accept");
                return -4;
            }
            /* TODO 性能
             * 只支持一个链接，并发量不足
             * */
            while (1)
            {
                FD_ZERO(&rfds);
                //FD_SET(0,&rfds);
                maxfd = 0;
                FD_SET(fd, &rfds);
                if (maxfd < fd)
                {
                    maxfd = fd;
                }
                tv.tv_sec = 5;
                tv.tv_usec = 0;
                /* TODO 性能
                 * select 其实可以同时处理acceptfd和其他fd，提高并发量
                 * */
                retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);
                if (retval == -1)
                {
                    printf("failed to select");
                    break;
                }
                else if (retval == 0)
                {
                    continue;
                }
                else
                {
                    if (FD_ISSET(fd, &rfds))
                    {
                        char buff[1024];
                        memset(buff, 0, sizeof buff);
                        int buff_len = recv(fd, buff, sizeof buff, 0);
                        if (buff_len < 0)
                        {
                            cerr << "failed to recv" << endl;
                            close(fd);
                            break;
                        }
                        else
                        {
                            string get = buff;
                            //cout<<"get:"<<get<<endl;
                            from_queue_->Push(make_pair(get, fd));
                            //puts("pass push");
                            //if(get == "quit\n")
                            //{
                            //	puts("return");
                            //	return 0;
                            //}

                            break;
                        }
                    }
                }
            }
        }
    }
    int GetStatus() { return status_; }
};

#endif
