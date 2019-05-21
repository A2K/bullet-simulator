#pragma once

#include "Collection.h"

#include <list>


template<typename T>
class List : public Collection<std::list<T>, T>
{
public:

  using Collection<std::list<T>, T>::Collection;

};
