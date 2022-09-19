set(config_SRCS
  config.cc
  init.cc
  )

add_library(coserv_config ${config_SRCS})
target_link_libraries(coserv_config coserv_net coserv_base)

install(TARGETS coserv_config DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/coserv/config)

