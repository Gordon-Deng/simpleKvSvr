#ifndef WORKER_H
#define WORKER_H
class Worker{
public:
    /* XXX 代码容错性
     *   如果函数有可能发生错误，尽量让caller感知到错误，做一些容错逻辑
     *   微信这边一般是使用返回码，不用异常，具体原因可以了解下返回码和异常对性能的影响"
     * */
    virtual int Process() = 0;
	//virtual ~Worker() = 0;
};
#endif
