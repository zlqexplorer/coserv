#include "coserv/coroutine/cothread.h"
#include <functional>
#include ""
#include "coserv/net/EventLoopThreadPool.h"

extern coserv::EventLoopThreadPool* eventPool;

int cothread(threadFunc func,void* arg){
    coserv::EventLoop::Functor cb=std::bind(func,arg);
    coserv::EventLoop* loop=eventPool->getNextLoop();
    loop->queueInLoop(cb);
    return 0;
}