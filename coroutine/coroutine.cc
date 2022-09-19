#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <atomic>
#include "coserv/coroutine/coroutine.h"
#include "coserv/base/log/Logging.h"

namespace coserv {

static thread_local Coroutine* t_main_coroutine = nullptr;

static thread_local Coroutine* t_cur_coroutine = nullptr;

static thread_local int t_coroutine_count = 0;

static thread_local int t_cur_coroutine_id = 0;

static thread_local bool t_enable_coroutine_swap = true;

int getCoroutineIndex() {
  return t_cur_coroutine_id;
}

void CoFunction(Coroutine* co) {

  if (co!= nullptr) {
    co->setIsInCoFunc(true);

    co->m_call_back();

    co->setIsInCoFunc(false);
  }

  Coroutine::Yield();
}

void Coroutine::SetCoroutineSwapFlag(bool value) {
  t_enable_coroutine_swap = value;
}

bool Coroutine::GetCoroutineSwapFlag() {
  return t_enable_coroutine_swap;
}

Coroutine::Coroutine() {
  m_cor_id = t_cur_coroutine_id++;
  ++t_coroutine_count;
  memset(&m_coctx, 0, sizeof(m_coctx));
}

Coroutine::Coroutine(int size) : m_stack_size(size) {

  if (t_main_coroutine == nullptr) {
    t_main_coroutine = new Coroutine();
  }
  // assert(t_main_coroutine != nullptr);

  m_stack_sp =  reinterpret_cast<char*>(malloc(m_stack_size));
  if (!m_stack_sp) {
    LOG_ERROR << "start server error. malloc stack return nullptr";
    Exit(0);
  }
  // assert(m_stack_sp != nullptr);

  m_cor_id = t_cur_coroutine_id++;
  ++t_coroutine_count;
  // LOG_DEBUG << "coroutine[null callback] created, id[" << m_cor_id << "]";
}

Coroutine::Coroutine(int size, char* stack_ptr) {
  if (t_main_coroutine == nullptr) {
    t_main_coroutine = new Coroutine();
  }

  assert(stack_ptr);
  m_stack_size = size;
  m_stack_sp = stack_ptr;
  m_cor_id = t_cur_coroutine_id++;
  ++t_coroutine_count;
}

Coroutine::Coroutine(int size, std::function<void()> cb)
  : m_stack_size(size) {

  if (t_main_coroutine == nullptr) {
    t_main_coroutine = new Coroutine();
  }
  // assert(t_main_coroutine != nullptr);

  m_stack_sp =  reinterpret_cast<char*>(malloc(m_stack_size));
  if (!m_stack_sp) {
    LOG_ERROR << "start server error. malloc stack return nullptr";
    Exit(0);
  }
  // assert(m_stack_sp != nullptr);

  setCallBack(cb);
  m_cor_id = t_cur_coroutine_id++;
  ++t_coroutine_count;
  // LOG_DEBUG << "coroutine created, id[" << m_cor_id << "]";
}

bool Coroutine::setCallBack(std::function<void()> cb) {

  if (this == t_main_coroutine) {
    LOG_ERROR << "main coroutine can't set callback";
    return false;
  }
  if (m_is_in_cofunc) {
    LOG_ERROR << "this coroutine is in CoFunction";
    return false;
  }

  m_call_back = cb;

  // assert(m_stack_sp != nullptr);

  char* top = m_stack_sp + m_stack_size;
  // first set 0 to stack
  // memset(&top, 0, m_stack_size);

  top = reinterpret_cast<char*>((reinterpret_cast<unsigned long>(top)) & -16LL);

  memset(&m_coctx, 0, sizeof(m_coctx));

  m_coctx.regs[kRSP] = top;
  m_coctx.regs[kRBP] = top;
  m_coctx.regs[kRETAddr] = reinterpret_cast<char*>(CoFunction); 
  m_coctx.regs[kRDI] = reinterpret_cast<char*>(this);

  return true;

}

Coroutine::~Coroutine() {
  --t_coroutine_count;

  if (m_stack_sp != nullptr) {
    free(m_stack_sp);
    m_stack_sp = nullptr;
  }
  LOG_DEBUG << "coroutine[" << m_cor_id << "] die";
}

Coroutine* Coroutine::GetCurrentCoroutine() {
  if (t_cur_coroutine == nullptr) {
    t_main_coroutine = new Coroutine();
    t_cur_coroutine = t_main_coroutine;
  }
  return t_cur_coroutine;
}

bool Coroutine::IsMainCoroutine() {
  if (t_main_coroutine == nullptr || t_cur_coroutine == t_main_coroutine) {
    return true;
  }
  return false;
}

/********
让出执行权,切换到主协程
********/
void Coroutine::Yield() {
  if (!t_enable_coroutine_swap) {
    LOG_ERROR << "can't yield, because disable coroutine swap";
    return;
  }
  if (t_main_coroutine == nullptr) {
    LOG_ERROR << "main coroutine is nullptr";
    return;
  }

  if (t_cur_coroutine == t_main_coroutine) {
    LOG_ERROR << "current coroutine is main coroutine";
    return;
  }
  Coroutine* co = t_cur_coroutine;
  t_cur_coroutine = t_main_coroutine;
  coctx_swap(&(co->m_coctx), &(t_main_coroutine->m_coctx));
  // LOG_DEBUG << "swap back";
}

/********
取得执行权,从主协程切换到目标协程
********/
void Coroutine::Resume(Coroutine* co) {

  if (t_cur_coroutine != t_main_coroutine) {
    LOG_ERROR << "swap error, current coroutine must be main coroutine";
    return;
  }

  if (t_main_coroutine == nullptr) {
    LOG_ERROR << "main coroutine is nullptr";
    return;
  }
  if (co == nullptr) {
    LOG_ERROR << "pending coroutine is nullptr";
    return;
  }

  if (t_cur_coroutine == co) {
    LOG_DEBUG << "current coroutine is pending cor, need't swap";
    return;
  }
  t_cur_coroutine = co;

  coctx_swap(&(t_main_coroutine->m_coctx), &(co->m_coctx));
  // LOG_DEBUG << "swap back";

}

}
