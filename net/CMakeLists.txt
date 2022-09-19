include(CheckFunctionExists)

check_function_exists(accept4 HAVE_ACCEPT4)
if(NOT HAVE_ACCEPT4)
  set_source_files_properties(SocketsOps.cc PROPERTIES COMPILE_FLAGS "-DNO_ACCEPT4")
endif()

set(net_SRCS
  Channel.cc
  EventLoop.cc
  EventLoopThread.cc
  EventLoopThreadPool.cc
  Poller.cc
  EPollPoller.cc
  SocketsOps.cc
  Timer.cc
  TimerQueue.cc
  )

add_library(coserv_net ${net_SRCS})
target_link_libraries(coserv_net coserv_base)

#add_library(coserv_net_cpp11 ${net_SRCS})
#target_link_libraries(coserv_net_cpp11 coserv_base_cpp11)
#set_target_properties(coserv_net_cpp11 PROPERTIES COMPILE_FLAGS "-std=c++0x")

install(TARGETS coserv_net DESTINATION lib)
#install(TARGETS coserv_net_cpp11 DESTINATION lib)

set(HEADERS
  Callbacks.h
  Channel.h
  Endian.h
  EventLoop.h
  EventLoopThread.h
  EventLoopThreadPool.h
  TimerId.h
  )
install(FILES ${HEADERS} DESTINATION include/coserv/net)