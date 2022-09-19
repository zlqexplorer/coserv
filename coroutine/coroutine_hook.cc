#include <assert.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include <unordered_map>
#include <functional>
#include "coserv/net/SocketsOps.h"
#include "coserv/base/Mutex.h"
#include "coserv/coroutine/coroutine_hook.h"
#include "coserv/coroutine/coroutine.h"
#include "coserv/net/Channel.h"
#include "coserv/net/Eventloop.h"
#include "coserv/net/Callbacks.h"
#include "coserv/net/TimerId.h"
#include "coserv/base/log/Logging.h"
#include "coserv/base/config/config.h"

#define HOOK_SYS_FUNC(name) name##_fun_ptr_t g_sys_##name##_fun = (name##_fun_ptr_t)dlsym(RTLD_NEXT, #name);


HOOK_SYS_FUNC(accept);
HOOK_SYS_FUNC(read);
HOOK_SYS_FUNC(write);
HOOK_SYS_FUNC(connect);
HOOK_SYS_FUNC(socket);
HOOK_SYS_FUNC(close);
HOOK_SYS_FUNC(sleep);

namespace coserv {

extern Config* gConfig;

net::Channel** FdChannelMap;

static bool g_hook = true;

void SetHook(bool value) {
	g_hook = value;
}

int socket_hook(int domain, int type, int protocol){
  int sockfd=createNonblockingOrDie(domain);
  assert(FdChannelMap[fd]==nullptr);
  FdChannelMap[fd]=new net::Channel(sockfd,EventLoop::getEventLoopOfCurrentThread());
  return sockfd;
}

ssize_t read_hook(int fd, void *buf, size_t count) {
	LOG_DEBUG << "this is hook read";
  if (Coroutine::IsMainCoroutine()) {
    LOG_DEBUG << "hook disable, call sys read func";
    return g_sys_read_fun(fd, buf, count);
  }
  
  ssize_t n = g_sys_read_fun(fd, buf, count);    //Nonblock
  if (n > 0) {
    return n;
  } 

	net::EventLoop* loop=EventLoop::getEventLoopOfCurrentThread();

  net::Channel* channel=FdChannelMap[fd];
  if(!channel){
    channel=FdChannelMap[fd]=new net::Channel(sockfd,loop); 
  }
  assert(loop==channel->ownerLoop());

  channel->setCallback(std::bind(&Coroutine::Resume,Coroutine::GetCurrentCoroutine()));
	
  channel->enableReading();

	LOG_DEBUG << "read func to yield";

	Coroutine::Yield();
  
  LOG_DEBUG << "read func yield back";

	channel->disableReadingDelay();

  LOG_DEBUG << "Remove new channel" ;

	return g_sys_read_fun(fd, buf, count);
}

int accept_hook(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	LOG_DEBUG << "this is hook accept";
  if (Coroutine::IsMainCoroutine()) {
    LOG_DEBUG << "hook disable, call sys accept func";
    return g_sys_accept_fun(sockfd, addr, addrlen);
  }

  int n = g_sys_accept_fun(sockfd, addr, addrlen);  //Nonblock
  if (n > 0) {
    return n;
  } 

  net::EventLoop* loop=EventLoop::getEventLoopOfCurrentThread();

  net::Channel* channel=FdChannelMap[sockfd];
  assert(loop==channel->ownerLoop());

  channel->setCallback(std::bind(&Coroutine::Resume,Coroutine::GetCurrentCoroutine()));
	
  channel->enableReading();

	LOG_DEBUG << "accept func to yield";

	Coroutine::Yield();
  
  LOG_DEBUG << "accept func yield back";

	channel->disableReadingDelay();

  LOG_DEBUG << "Remove new channel" ;
	
	return g_sys_accept_fun(sockfd, addr, addrlen);
}

ssize_t write_hook(int fd, const void *buf, size_t count) {
	LOG_DEBUG << "this is hook write";
  if (Coroutine::IsMainCoroutine()) {
    LOG_DEBUG << "hook disable, call sys write func";
    return g_sys_write_fun(fd, buf, count);
  }
	
  ssize_t n = g_sys_write_fun(fd, buf, count);
  if (n > 0) {
    return n;
  }

	net::EventLoop* loop=EventLoop::getEventLoopOfCurrentThread();

  net::Channel* channel=FdChannelMap[fd];
  if(!channel){
    channel=FdChannelMap[fd]=new net::Channel(sockfd,loop); 
  }
  assert(loop==channel->ownerLoop());

  channel->setCallback(std::bind(&Coroutine::Resume,Coroutine::GetCurrentCoroutine()));
	
  channel->enableWriting();

	LOG_DEBUG << "write func to yield";
	Coroutine::Yield();

	LOG_DEBUG << "write func yield back";

  channel->disableWritingDelay();

  LOG_DEBUG << "Remove new channel" ;

	return g_sys_write_fun(fd, buf, count);
}

int connect_hook(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	LOG_DEBUG << "this is hook connect";
  if (Coroutine::IsMainCoroutine()) {
    LOG_DEBUG << "hook disable, call sys connect func";
    return g_sys_connect_fun(sockfd, addr, addrlen);
  }

  int n = g_sys_connect_fun(sockfd, addr, addrlen);
  if (n == 0) {
    LOG_DEBUG << "direct connect succ, return";
    return n;
  } else if (errno != EINPROGRESS) {
		LOG_DEBUG << "connect error and errno is't EINPROGRESS, errno=" << errno <<  ",error=" << strerror(errno);
    return n;
  }

	net::EventLoop* loop=EventLoop::getEventLoopOfCurrentThread();

  net::Channel* channel=FdChannelMap[fd];
  assert(loop==channel->ownerLoop());

  channel->setCallback(std::bind(&Coroutine::Resume,Coroutine::GetCurrentCoroutine()));
	
  channel->enableWriting();

	Coroutine* cur_cor = Coroutine::GetCurrentCoroutine();
	bool is_timeout = false;		

  net::TimerCallback timeout_cb = [cur_cor, &is_timeout](){
		LOG_DEBUG << "Connect out of time";
		is_timeout = true;
		Coroutine::Resume(cur_cor);
  };

  net::TimerId timer_id=loop->runAfter(static_cast<double>(gConfig->m_max_connect_timeout),timeout_cb);

  Coroutine::Yield();

  loop->removeChannel(channel);  // Just in case that sockfd connected 
  FdChannelMap[fd]=nullptr;      // in one thread and used in another thread.
  loop->cancel(timer_id);

	n = g_sys_connect_fun(sockfd, addr, addrlen);
	if ((n < 0 && errno == EISCONN) || n == 0) {
		LOG_DEBUG << "connect succ";
		return 0;
	}

	if (is_timeout) {
    ErrorLog << "connect error,  timeout[ " << gConfig->m_max_connect_timeout << "ms]";
		errno = ETIMEDOUT;
	} 

	LOG_DEBUG << "connect error and errno=" << errno <<  ", error=" << strerror(errno);
	return -1;
}

unsigned int sleep_hook(unsigned int seconds) {
	LOG_DEBUG << "this is hook sleep";
  if (Coroutine::IsMainCoroutine()) {
    LOG_DEBUG << "hook disable, call sys sleep func";
    return g_sys_sleep_fun(seconds);
  }

	Coroutine* cur_cor = Coroutine::GetCurrentCoroutine();
	bool is_timeout = false;

	net::TimerCallback timeout_cb = [cur_cor, &is_timeout](){
		LOG_DEBUG << "onTime, now resume sleep cor";
		is_timeout = true;
		Coroutine::Resume(cur_cor);
  };

  net::EventLoop* loop=EventLoop::getEventLoopOfCurrentThread();

  loop->runAfter(static_cast<double>(seconds),timeout_cb);

	LOG_DEBUG << "now to yield sleep";
	
	Coroutine::Yield();

	return 0;
}

int close_hook(int fd){
  net::Channel* channel=FdChannelMap[fd];
  if(channel){
    net::EventLoop* loop=channel->loop;
    if(loop->hasChannel(channel)){
      loop->removeChannel(channel);
    }
    FdChannelMap[fd]=nullptr;
    delete channel;
  }
  g_sys_close_fun(fd);
}

}  //namespace coserv


extern "C" {

int socket(int domain, int type, int protocol){
  if (!coserv::g_hook || !coserv::Coroutine::GetCoroutineSwapFlag()) {
		return g_sys_socket_fun(sockfd, addr, addrlen);
	} else {
		return coserv::socket_hook(sockfd, addr, addrlen);
	}
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	if (!coserv::g_hook || !coserv::Coroutine::GetCoroutineSwapFlag()) {
		return g_sys_accept_fun(sockfd, addr, addrlen);
	} else {
		return coserv::accept_hook(sockfd, addr, addrlen);
	}
}

ssize_t read(int fd, void *buf, size_t count) {
	if (!coserv::g_hook || !coserv::Coroutine::GetCoroutineSwapFlag()) {
		return g_sys_read_fun(fd, buf, count);
	} else {
		return coserv::read_hook(fd, buf, count);
	}
}

ssize_t write(int fd, const void *buf, size_t count) {
	if (!coserv::g_hook || !coserv::Coroutine::GetCoroutineSwapFlag()) {
		return g_sys_write_fun(fd, buf, count);
	} else {
		return coserv::write_hook(fd, buf, count);
	}
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	if (!coserv::g_hook || !coserv::Coroutine::GetCoroutineSwapFlag()) {
		return g_sys_connect_fun(sockfd, addr, addrlen);
	} else {
		return coserv::connect_hook(sockfd, addr, addrlen);
	}
}

unsigned int sleep(unsigned int seconds) {
	if (!coserv::g_hook || !coserv::Coroutine::GetCoroutineSwapFlag()) {
		return g_sys_sleep_fun(seconds);
	} else {
		return coserv::sleep_hook(seconds);
	}
}

int close(int fd){
  if (!coserv::g_hook || !coserv::Coroutine::GetCoroutineSwapFlag()) {
		return g_sys_close_fun(fd, buf, count);
	} else {
		return coserv::close_hook(fd, buf, count);
	}
}

}