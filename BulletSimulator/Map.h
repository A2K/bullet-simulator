#pragma once

#include <map>
#include <functional>


template<typename K, typename T, template<typename, typename> typename Base = std::map>
class Map : public Base<K, T>
{
public:

  using Base<K, T>::Base;

  T& Add(const K& key, const T& value)
  {    
    return (*this)[key] = value;
  }

  T* Get(const K& key)
  {
    auto iter = this->find(key);
    return (iter == this->end()) ? nullptr : &iter->second;
  }

  bool Remove(const K& key)
  {
    auto iter = this->find(key);

    if (iter != this->end())
    {
      this->erase(iter);
      return true;
    }

    return false;
  }
  
  typedef std::function<void(const K& key, T& value)> ForEachDelegate;

  void ForEach(ForEachDelegate callback)
  {
    for (auto& pair : *this)
      callback(pair.first, pair.second);
  }

};
