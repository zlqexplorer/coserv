set(coroutine_SRCS
  cothread.cc
  coroutine_pool
  coroutine_hook
  coroutine
  coctx_swap
  )

add_library(coserv_coroutine ${coroutine_SRCS})
target_link_libraries(coserv_coroutine coserv_net coserv_base)

#add_library(coserv_base_cpp11 ${base_SRCS})
#target_link_libraries(coserv_base_cpp11 pthread rt)
#set_target_properties(coserv_base_cpp11 PROPERTIES COMPILE_FLAGS "-std=c++0x")

install(TARGETS coserv_coroutine DESTINATION lib)
#install(TARGETS coserv_coroutine DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/coserv/coroutine)