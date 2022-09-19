#ifndef COSERV_BASE_COPYABLE_H
#define COSERV_BASE_COPYABLE_H

namespace coserv
{

/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
class copyable
{
 protected:
  copyable() = default;
  ~copyable() = default;
};

}  // namespace coserv

#endif  // COSERV_BASE_COPYABLE_H
