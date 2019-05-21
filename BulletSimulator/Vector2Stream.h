#pragma once

#include "Vector2.h"

#include <ostream>


template<typename T>
std::ostream& operator<<(std::ostream& stream, const Vector2<T>& v2)
{
  stream << "(X=" << v2.x << ",Y=" << v2.y << ")";
  return stream;
}

template<typename T, typename T2>
Vector2<T> operator*(const T2& scalar, const Vector2<T>& rhs)
{
  return { rhs.x * scalar, rhs.y * scalar };
}
