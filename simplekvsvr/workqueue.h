#ifndef WORKQUEUE_H
#define WORKQUEUE_H

#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
using namespace std;
template <class T>
class workqueue{
public:
    void Push(T);
    T Pop();
private:
    queue<T> queue_;
    condition_variable not_empty_;
    mutex mutex_;
};
template <class T>
void workqueue<T>::Push(T element){
    unique_lock<mutex> lock(mutex_);
    queue_.push(element);
    lock.unlock();
    not_empty_.notify_one();
}
template <class T>
T workqueue<T>::Pop(){
    unique_lock<mutex> lock(mutex_);
    while(queue_.empty())
        not_empty_.wait(lock);
    T ret=queue_.front();
    queue_.pop();
    return ret;
}
#endif
