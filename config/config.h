#ifndef COSERV_COMM_CONFIG_H
#define COSERV_COMM_CONFIG_H

#include <tinyxml/tinyxml.h>
#include <string>
#include <memory>
#include <map>

namespace coserv {

class Config {

 public:
  Config(const char* file_path);

  ~Config();

  void readConf();

 public:

  // coroutine params
  int m_cor_stack_size {0};
  int m_cor_pool_size {0};

  int m_max_connect_timeout {0};    // ms
  int m_iothread_num {0};

  std::string name;


 private:
  std::string m_file_path;
  TiXmlDocument* m_xml_file;

};


}

#endif // COSERV_COMM_CONFIG_H