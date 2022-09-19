#include <tinyxml/tinyxml.h>
#include <assert.h>
#include <stdio.h>
#include <memory>
#include "coserv/base/config.h"
#include "coserv/base/log/logging.h"
#include "coserv/base/log/AsyncLogging.h"


namespace coserv {

extern Logger::LogLevel g_logLevel;
extern AsyncLogging* g_asyncLog;

namespace detail {
void asyncOutput(const char* msg, int len)
{
  g_asyncLog->append(msg, len);
}
}

Config::Config(const char* file_path) : m_file_path(std::string(file_path)) {
  m_xml_file = new TiXmlDocument();
  bool rt = m_xml_file->LoadFile(file_path);
  if (!rt) {
    printf("start coserv server error! read conf file [%s] error info: [%s], errorid: [%d], error_row_column:[%d row %d column]\n", 
      file_path, m_xml_file->ErrorDesc(), m_xml_file->ErrorId(), m_xml_file->ErrorRow(), m_xml_file->ErrorCol());
    exit(0);
  }
}

void Config::readConf() {
  TiXmlElement* root = m_xml_file->RootElement();

  TiXmlElement* log_node = root->FirstChildElement("log");
  if (!log_node) {
    printf("start coserv server error! read config file [%s] error, cannot read [log] xml node\n", m_file_path.c_str());
    exit(0);
  }

  if (!log_node->FirstChildElement("log_file_base_name") || !log_node->FirstChildElement("log_file_base_name")->GetText()) {
    printf("start coserv server error! read config file [%s] error, cannot read [log.log_file_base_name] xml node\n", m_file_path.c_str());
    exit(0);
  }

  std::string log_name(log_node->FirstChildElement("log_file_base_name")->GetText());

  if (!log_node->FirstChildElement("log_max_file_size") || !log_node->FirstChildElement("log_max_file_size")->GetText()) {
    printf("start coserv server error! read config file [%s] error, cannot read [log.log_max_file_size] xml node\n", m_file_path.c_str());
    exit(0);
  }

  off_t rollsize=std::atoi(log_node->FirstChildElement("log_max_file_size")->GetText());
  rollsize*=1024*1024;

  if (!log_node->FirstChildElement("log_level") || !log_node->FirstChildElement("log_level")->GetText()) {
    printf("start coserv server error! read config file [%s] error, cannot read [log.log_level] xml node\n", m_file_path.c_str());
    exit(0);
  }

  g_logLevel=Logger::stringToLevel(log_node->FirstChildElement("log_level")->GetText());

  if (!log_node->FirstChildElement("log_sync_inteval") || !log_node->FirstChildElement("log_sync_inteval")->GetText()) {
    printf("start coserv server error! read config file [%s] error, cannot read [log.log_sync_inteval] xml node\n", m_file_path.c_str());
    exit(0);
  }

  int flushInterval=std::atoi(log_node->FirstChildElement("log_sync_inteval")->GetText());

  g_asyncLog=new AsyncLogging(log_name,rollsize,flushInterval);

  g_asyncLog->start();

  Logger::setOutput(detail::asyncOutput);

  TiXmlElement* coroutine_node = root->FirstChildElement("coroutine");
  if (!coroutine_node) {
    printf("start coserv server error! read config file [%s] error, cannot read [coroutine] xml node\n", m_file_path.c_str());
    exit(0);
  }

  if (!coroutine_node->FirstChildElement("coroutine_stack_size") || !coroutine_node->FirstChildElement("coroutine_stack_size")->GetText()) {
    printf("start coserv server error! read config file [%s] error, cannot read [coroutine.coroutine_stack_size] xml node\n", m_file_path.c_str());
    exit(0);
  }

  if (!coroutine_node->FirstChildElement("coroutine_pool_size") || !coroutine_node->FirstChildElement("coroutine_pool_size")->GetText()) {
    printf("start coserv server error! read config file [%s] error, cannot read [coroutine.coroutine_pool_size] xml node\n", m_file_path.c_str());
    exit(0);
  }

  int cor_stack_size = std::atoi(coroutine_node->FirstChildElement("coroutine_stack_size")->GetText());
  m_cor_stack_size = 1024 * cor_stack_size;
  m_cor_pool_size = std::atoi(coroutine_node->FirstChildElement("coroutine_pool_size")->GetText());

  if (!root->FirstChildElement("max_connect_timeout") || !root->FirstChildElement("max_connect_timeout")->GetText()) {
    printf("start coserv server error! read config file [%s] error, cannot read [max_connect_timeout] xml node\n", m_file_path.c_str());
    exit(0);
  }
  int max_connect_timeout = std::atoi(root->FirstChildElement("max_connect_timeout")->GetText());
  m_max_connect_timeout = max_connect_timeout * 1000;

  if (!root->FirstChildElement("eventloop_num") || !root->FirstChildElement("eventloop_num")->GetText()) {
    printf("start coserv server error! read config file [%s] error, cannot read [eventloop_num] xml node\n", m_file_path.c_str());
    exit(0);
  }
  name = root->FirstChildElement("max_connect_timeout")->GetText();

  if (!root->FirstChildElement("iothread_num") || !root->FirstChildElement("iothread_num")->GetText()) {
    printf("start coserv server error! read config file [%s] error, cannot read [iothread_num] xml node\n", m_file_path.c_str());
    exit(0);
  }

  m_iothread_num = std::atoi(root->FirstChildElement("iothread_num")->GetText());

  char buff[512];
  sprintf(buff, "read config from file [%s]: [coroutine_stack_size: %d KB], [coroutine_pool_size: %d], [max_connect_timeout: %d s], "
      "[eventloop_num: %s] ","[iothread_num: %d]",
      m_file_path.c_str(), cor_stack_size, m_cor_pool_size,max_connect_timeout, name.c_str(), m_iothread_num);

  std::string s(buff);
  LOG_INFO << s;

}

Config::~Config() {
  if (m_xml_file) {
    delete m_xml_file;
    m_xml_file = NULL;
  }
}


}