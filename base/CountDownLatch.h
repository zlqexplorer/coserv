// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef COSERV_BASE_COUNTDOWNLATCH_H
#define COSERV_BASE_COUNTDOWNLATCH_H

#include "coserv/base/Condition.h"
#include "coserv/base/Mutex.h"

namespace coserv
{

class CountDownLatch : noncopyable
{
 public:

  explicit CountDownLatch(int count);

  void wait();

  void countDown();

  int getCount() const;

 private:
  mutable MutexLock mutex_;
  Condition condition_ GUARDED_BY(mutex_);
  int count_ GUARDED_BY(mutex_);
};

}  // namespace coserv
#endif  // COSERV_BASE_COUNTDOWNLATCH_H
