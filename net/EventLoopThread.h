// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/coserv/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef COSERV_NET_EVENTLOOPTHREAD_H
#define COSERV_NET_EVENTLOOPTHREAD_H

#include "coserv/base/Condition.h"
#include "coserv/base/Mutex.h"
#include "coserv/base/Thread.h"

namespace coserv
{
namespace net
{

class EventLoop;

class EventLoopThread : noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const string& name = string());
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();

  EventLoop* loop_ GUARDED_BY(mutex_);
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_ GUARDED_BY(mutex_);
  ThreadInitCallback callback_;
};

}  // namespace net
}  // namespace coserv

#endif  // COSERV_NET_EVENTLOOPTHREAD_H

