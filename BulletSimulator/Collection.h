#pragma once

#include <functional>


template<class Base, typename T>
class Collection : public Base
{
public:

  using Base::Base;

  const T& Add(const T& element)
  {
    return *this->insert(this->end(), element);
  }

  bool Remove(const T& element)
  {
    auto iter = this->find(element);
    if (iter == this->end()) return false;
    this->erase(iter);
    return true;
  }

  bool Contains(const T& element) const
  {
    return this->find(element) != this->end();
  }

  template<typename R = Set<T>>
  R ElementsIn(const Collection& other)
  {
    R res;
    for (const T& value : *this)
      if (other.Contains(value))
        res.Add(value);
    return res;
  }

  template<typename R = Set<T>>
  R ElementsNotIn(const Collection& other) const
  {
    R res;
    for (const auto& value : *this)
      if (!other.Contains(value))
        res.Add(value);
    return res;
  }

  Collection& AddAll(const Collection& other)
  {
    this->insert(other.begin(), other.end());
    return *this;
  }

  Collection& RemoveAll(const Collection& other)
  {
    for (const auto& value : other)
    {
      if (!this->size()) return;
      Remove(value);
    }
    return *this;
  }

  Collection& operator+=(const T& element)
  {
    Add(element);
    return *this;
  }

  Collection& operator+=(const Collection& other)
  {
    AddAll(other);
    return *this;
  }

  Collection& operator-=(const T& element)
  {
    Remove(element);
    return *this;
  }

  Collection& operator-=(const Collection& other)
  {
    RemoveAll(other);
    return *this;
  }

  const T& First() const
  {
    return *this->begin();
  }

  const T& Last() const
  {
    return *this->rbegin();
  }

  bool PopFirstIf(std::function<bool(const T&)> predicate)
  {
    if (!this->size()) return false;
    auto iter = this->begin();
    if (predicate(*iter))
    {
      this->erase(iter);
      return true;
    }
    return false;
  }

  bool PopLastIf(std::function<bool(const T&)> predicate)
  {
    if (!this->size()) return false;
    auto iter = --this->end();
    if (predicate(*iter))
    {
      this->erase(iter);
      return true;
    }
    return false;
  }

  T PopFirst()
  {
    if (!this->size()) return T();
    auto iter = this->begin();
    T value = *iter;
    this->erase(iter);
    return value;
  }

  T PopLast()
  {
    if (!this->size()) return T();
    auto iter = --this->end();
    T value = *iter;
    this->erase(iter);
    return value;
  }

  template<typename T2>
  T2 As()
  {
    return T2(this->begin(), this->end());
  }

};
