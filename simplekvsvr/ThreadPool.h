#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "iostream"
#include "functional"
#include "vector"
#include "stdio.h"
#include "thread"
#include "workqueue.h"
#include "Worker.h"
using namespace std;

class ThreadPool{
public:
    ThreadPool(int thread_num,Worker *thread_worker);
    void Start();
    void Stop();
    void Process();
private:
    Worker *thread_worker_;
    vector<thread*> threads_;
    int thread_num_;
};

ThreadPool::ThreadPool(int thread_num,Worker *thread_worker){
    thread_num_=thread_num;
    thread_worker_=thread_worker;
}
/*----------------------------------------
void ThreadPool::Process(){
    while(1){
        Task t=wq.pop();
        t();
    }
}
-----------------------------------------*/
void ThreadPool::Start(){
    for(int i=0;i<thread_num_;++i){
        // thread t(bind(&ThreadPool::Process,this));
        thread *t=new thread(bind(&Worker::Process,thread_worker_));
        threads_.push_back(t);
    }
}
/*----------------------------------------
void ThreadPool::add(Task task){
    wq.push(task);
}
-----------------------------------------*/
void ThreadPool::Stop(){
    for(int i=0;i<thread_num_;++i){
        threads_[i]->join();
        delete threads_[i];
    }
}

#endif
