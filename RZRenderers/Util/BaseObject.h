//=============================================================================
// BaseObject.h - Implements ref count and makes the object non-copyable
// Reza Nourai, 2016
//=============================================================================
#pragma once

#include <atomic>

template <typename Interface>
class BaseObject : public Interface
{
public:
  virtual void AddRef() override
  {
    ++ref_count_;
  }

  virtual void Release() override
  {
    if (--ref_count_ == 0)
    {
      delete this;
    }
  }

protected:
  BaseObject() {}
  virtual ~BaseObject() {}

private:
  BaseObject(const BaseObject&) = delete;
  BaseObject& operator= (const BaseObject&) = delete;

private:
  std::atomic<int> ref_count_ = ATOMIC_VAR_INIT(1);
};
