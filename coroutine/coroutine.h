#ifndef COSERV_COROUTINE_COROUTINE_H
#define COSERV_COROUTINE_COROUTINE_H

#include <memory>
#include <functional>
#include "coserv/coroutine/coctx.h"

namespace coserv {

int getCoroutineIndex();

class Coroutine {

 public:
  typedef std::shared_ptr<Coroutine> ptr;

  Coroutine(int size);

  Coroutine(int size, std::function<void()> cb);

  Coroutine(int size, char* stack_ptr);

  ~Coroutine();

  bool setCallBack(std::function<void()> cb); 

  int getCorId() const {
    return m_cor_id;
  }

  void setIsInCoFunc(const bool v) {
    m_is_in_cofunc = v;
  }

  bool getIsInCoFunc() const {
    return m_is_in_cofunc;
  }

 public:
  static void Yield();

  static void Resume(Coroutine* cor);

  static Coroutine* GetCurrentCoroutine();

  static bool IsMainCoroutine();

  static void SetCoroutineSwapFlag(bool value);

  static bool GetCoroutineSwapFlag();

 private:
  Coroutine();

 private:
  int m_cor_id {0};       
  coctx m_coctx;      
  int m_stack_size {0};   
  char* m_stack_sp {nullptr};   
  bool m_is_in_cofunc {false};  

 public:
  std::function<void()> m_call_back;
};

}


#endif
