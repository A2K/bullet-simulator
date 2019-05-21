#pragma once

#include <cmath>


#pragma pack(push, 4)

template<typename T>
class Vector2
{
public:

  T x;
  T y;

public:

  Vector2() :
    x(0), y(0)
  {}

  Vector2(const T& v) :
    x(v), y(v)
  {}

  Vector2(const T& X, const T& Y) :
    x(X), y(Y)
  {}

  template<typename T2>
  Vector2(const T2& v) :
    x(v), y(v)
  {}

  template<typename T1, typename T2>
  Vector2(const T1& X, const T2& Y) :
    x(T(X)), y(T(Y))
  {}

  Vector2(const Vector2& v) :
    x(v.x), y(v.y)
  {}

  template<typename T2>
  Vector2(const Vector2<T2>& v) :
    x(v.x), y(v.y)
  {}

public:

  T Length() const
  {
    return std::sqrt(x * x + y * y);
  }

  Vector2 Normalized() const
  {
    T length = Length();
    return length > 0 ? (*this) / length : Vector2(0);
  }

  template<typename T2>
  Vector2 DirectionTo(const Vector2<T2>& target) const
  {
    return (target - *this).Normalized();
  }

  template<typename T2>
  T SquaredDistanceTo(const Vector2<T2>& other) const
  {
    T dx = this->x - other.x;
    T dy = this->y - other.y;
    return dx * dx + dy * dy;
  }

  template<typename T2>
  T DistanceTo(const Vector2<T2>& other) const
  {
    return std::sqrt(SquaredDistanceTo(other));
  }

  template<typename T2>
  T Cross(const Vector2<T2>& other) const
  {
    return x * other.y - y * other.x;
  }

  template<typename T2>
  T Dot(const Vector2<T2>& other) const
  {
    return x * other.x + y * other.y;
  }

  template<typename T2>
  Vector2<T2> Reflect(const Vector2<T2>& normal) const
  {
    return (*this) - normal * Dot(normal) * 2;
  }
  
  T Max() const
  {
    return x > y ? x : y;
  }

  T Min() const
  {
    return x < y ? x : y;
  }
  
public:

  template<typename T2>
  bool operator==(const Vector2<T2>& other) const
  {
    return x == other.x && y == other.y;
  }

  template<typename T2>
  bool operator<(const Vector2<T2>& other) const
  {
    return (x == other.x)
      ? y < other.y
      : x < other.x;
  }

  template<typename T2>
  Vector2 operator*(const Vector2<T2>& other) const
  {
    return { x * other.x, y * other.y };
  }

  template<typename T2>
  Vector2 operator*(const T2& scalar) const
  {
    return { x * scalar, y * scalar };
  }

  template<typename T2>
  Vector2 operator+(const Vector2<T2>& other) const
  {
    return { x + other.x, y + other.y };
  }

  template<typename T2>
  Vector2 operator+(const T2& other) const
  {
    return { x + other, y + other };
  }

  template<typename T2>
  Vector2 operator-(const Vector2<T2>& other) const
  {
    return { x - other.x, y - other.y };
  }

  template<typename T2>
  Vector2 operator-(const T2& other) const
  {
    return { x - other, y - other };
  }

  template<typename T2>
  Vector2 operator/(const Vector2<T2>& other) const
  {
    return { x / other.x, y / other.y };
  }

  template<typename T2>
  Vector2 operator/(const T2& value) const
  {
    return { x / value, y / value };
  }

public:

  template<typename T2>
  Vector2& operator=(const T2& value)
  {
    return *this = { value, value };
  }

  template<typename T2>
  Vector2& operator=(const Vector2<T2>& value)
  {
    return *this = { value.x, value.y };
  }

  template<typename T2>
  Vector2& operator+=(const Vector2<T2>& other)
  {
    return *this = { x + other.x, y + other.y };
  }

  template<typename T2>
  Vector2& operator+=(const T2& scalar)
  {
    return *this = { x + scalar, y + scalar };
  }

  template<typename T2>
  Vector2& operator-=(const Vector2<T2>& other)
  {
    return *this = { x - other.x, y - other.y };
  }

  template<typename T2>
  Vector2& operator-=(const T2& scalar)
  {
    return *this = { x - scalar, y - scalar };
  }

  template<typename T2>
  Vector2& operator/=(const Vector2<T2>& value)
  {
    return *this = { x / value.x, y / value.y };
  }

  template<typename T2>
  Vector2& operator/=(const T2& value)
  {
    return *this = { x / value, y / value };
  }

  template<typename T2>
  Vector2& operator*=(const Vector2<T2>& value)
  {
    return *this = { x * value.x, y * value.y };
  }

  template<typename T2>
  Vector2& operator*=(const T2& value)
  {
    return *this = { x * value, y * value };
  }
  
  Vector2 operator-() const
  {
    return { -x, -y };
  }
};

#pragma pack(pop)

template<typename T, typename T2, typename = std::enable_if_t<std::is_scalar<T>::value>>
inline Vector2<T> operator*(T value, const Vector2<T2>& vector)
{
  return vector * value;
}

template<typename T, typename T2, typename = std::enable_if_t<std::is_scalar<T>::value>>
inline Vector2<T> operator/(T value, const Vector2<T2>& vector)
{
  return { value / vector.x, value / vector.y };
}

template<typename T, typename T2, typename = std::enable_if_t<std::is_scalar<T>::value>>
inline Vector2<T> operator+(T value, const Vector2<T2>& vector)
{
  return vector + value;
}

template<typename T, typename T2, typename = std::enable_if_t<std::is_scalar<T>::value>>
inline Vector2<T> operator-(T value, const Vector2<T2>& vector)
{
  return { value - vector.x, value - vector.y };
}
