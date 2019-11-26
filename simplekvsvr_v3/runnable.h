
#ifndef MM_TRAINING_RUNNABLE_H
#define MM_TRAINING_RUNNABLE_H

namespace mminternpractice {

/**
 * 运行逻辑
 */
    class Runnable {
    public:
        /**
         * 析构函数
         */
        virtual ~Runnable() {}

        /**
         * 处理逻辑
         */
        virtual int Run() = 0;
    };

}  // mminternpractice

#endif
