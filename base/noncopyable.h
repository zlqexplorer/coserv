#ifndef COSERV_BASE_NONCOPYABLE_H
#define COSERV_BASE_NONCOPYABLE_H

namespace coserv
{

class noncopyable
{
 public:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

}  // namespace coserv

#endif  // COSERV_BASE_NONCOPYABLE_H
