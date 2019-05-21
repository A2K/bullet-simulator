#pragma once

#include "Collection.h"

#include <set>


template<typename T, class Cmp = std::less<T>>
class Set : public Collection<std::set<T, Cmp>, T>
{
public:

  using Collection<std::set<T, Cmp>, T>::Collection;

  Set Union(const Set& other) const
  {
    Set res(this->begin(), this->end());
    res.insert(other.begin(), other.end());
    return res;
  }

};
