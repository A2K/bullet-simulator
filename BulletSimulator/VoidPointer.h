#pragma once

#include <type_traits>


struct VoidPointer
{
  void* Ptr = nullptr;

  VoidPointer():
    Ptr(nullptr)
  {}

  VoidPointer(void* ptr):
    Ptr(ptr)
  {}

  VoidPointer(const VoidPointer& other):
    Ptr(other.Ptr)
  {}

  template<typename T>
  VoidPointer(T* ptr) :
    Ptr(ptr)
  {}

  template<typename T>
  T* Get() const
  {
    return reinterpret_cast<T*>(Ptr);
  }

  template<typename T>
  VoidPointer& operator=(const T* ptr)
  {
    Ptr = const_cast<void*>(reinterpret_cast<const void*>(ptr));
    return *this;
  }

  VoidPointer& operator=(const VoidPointer& other)
  {
    Ptr = other.Ptr;
    return *this;
  }

  VoidPointer& operator=(const std::nullptr_t& ptr)
  {
    Ptr = ptr;
    return *this;
  }

  template<typename T>
  operator T*() const
  {
    return reinterpret_cast<T*>(Ptr);
  }

  bool operator ==(const void* other) const
  {
    return Ptr == other;
  }

  template<typename T>
  bool operator ==(const T* other) const
  {
    return reinterpret_cast<T*>(Ptr) == other;
  }
  
  template<typename T>
  typename std::enable_if<std::is_integral<T>::value, bool>::type
  operator ==(const T other) const
  {
    return reinterpret_cast<void*>(other) == Ptr;
  }

  bool operator ==(const size_t& ptr) const
  {
    return Ptr == reinterpret_cast<void*>(ptr);
  }

  bool operator==(const std::nullptr_t& ptr) const
  {
    return Ptr == ptr;
  }

  bool operator<(const VoidPointer& other) const
  {
    return Ptr < other.Ptr;
  }

  bool operator>(const VoidPointer& other) const
  {
    return Ptr > other.Ptr;
  }

  bool operator<=(const VoidPointer& other) const
  {
    return Ptr <= other.Ptr;
  }

  bool operator>=(const VoidPointer& other) const
  {
    return Ptr >= other.Ptr;
  }

  bool operator!=(const VoidPointer& other) const
  {
    return Ptr != other.Ptr;
  }

  bool operator!() const
  {
    return Ptr == nullptr;
  }
  
  operator bool() const
  {
    return Ptr != nullptr;
  }

  template<typename T>
  T* operator*() const
  {
    return reinterpret_cast<T*>(Ptr);
  }
};
