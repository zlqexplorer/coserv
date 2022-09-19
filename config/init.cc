#include "coserv/base/config.h"
#include "coserv/base/log/logging.h"
#include "coserv/base/log/AsyncLogging.h"
#include "coserv/net/EventLoop.h"
#include "coserv/net/EventLoopThreadPool.h"

namespace coserv{
  Config* gconfig;
  Logger::LogLevel g_logLevel;
  AsyncLogging* g_asyncLog;
  EventLoopThreadPool* eventPool;


  int initAll(){
    gconfig=new Config("coserv/config/config_init.xml");
    gconfig->readConf();
    eventPool=new EventLoopThreadPool(new EventLoop(),gconfig->name);
    eventPool->setThreadNum(gconfig->m_iothread_num);
    eventPool->start();
    return 0;
  }

  int init=initAll();
}