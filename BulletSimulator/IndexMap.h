#pragma once

#include <functional>
#include <unordered_map>


template<
  typename K, 
  typename T, 
  template<typename, typename> typename Base = std::unordered_map>
class IndexMap : public Base<K, T>
{
public:

  using Base<K, T>::Base;

  T& Add(const T& item)
  {
    return (*this)[item.ID] = item;
  }

  T* Get(const K& id)
  {
    auto iter = this->find(id);
    return (iter == this->end()) ? nullptr : &iter->second;
  }

  bool Remove(const K& id)
  {
    auto iter = this->find(id);

    if (iter == this->end()) return false;
    
    this->erase(iter);
    return true;
  }

  template<typename T>
  void Convert(T iter)
  {
    for (auto& pair : *this)
      *(iter++) = pair.second;
  }

  typedef std::function<void(T& value)> ForEachDelegate;
  typedef std::function<void(T& value, size_t index)> ForEachIndexDelegate;

  void ForEach(ForEachDelegate callback)
  {
    for (auto& pair : *this)
      callback(pair.second);
  }

  size_t ForEach(ForEachIndexDelegate callback)
  {
    size_t index = 0;
    for (auto& pair : *this)
      callback(pair.second, index++);
    return index;
  }

};