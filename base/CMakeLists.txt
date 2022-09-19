set(base_SRCS
  log/AsyncLogging.cc
  Condition.cc
  CountDownLatch.cc
  CurrentThread.cc
  Date.cc
  Exception.cc
  FileUtil.cc
  log/LogFile.cc
  log/Logging.cc
  log/LogStream.cc
  ProcessInfo.cc
  Timestamp.cc
  Thread.cc
  ThreadPool.cc
  TimeZone.cc
  )

add_library(coserv_base ${base_SRCS})
target_link_libraries(coserv_base pthread rt)

install(TARGETS coserv_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/coserv/base)
