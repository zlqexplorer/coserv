#ifndef COSERV_COROUTINE_COUROUTINE_POOL_H
#define COSERV_COROUTINE_COUROUTINE_POOL_H

#include <vector>
#include "coserv/coroutine/coroutine.h"

namespace coserv {

class CoroutinePool {

 public:
  CoroutinePool(int pool_size, int stack_size = 1024 * 128);
  ~CoroutinePool();

  Coroutine::ptr getCoroutineInstanse();

  void returnCoroutine(Coroutine::ptr cor);

 private:
  int m_index {0};
  int m_pool_size {0};
  int m_stack_size {0};

  std::vector<std::pair<Coroutine::ptr, bool>> m_free_cors;

  char* m_memory_pool {NULL};

};


CoroutinePool* GetCoroutinePool();

}


#endif