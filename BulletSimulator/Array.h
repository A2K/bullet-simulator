#pragma once

#include "Collection.h"

#include <vector>


template<typename T>
class Array : public Collection<std::vector<T>, T>
{
public:

  using Collection<std::vector<T>, T>::Collection;

};
