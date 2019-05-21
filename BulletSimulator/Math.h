#pragma once


template<typename T> class Vector2;

template<typename T>
T Sign(const T& value)
{
  return value < 0 ? -1 : 1;
}

template<typename T>
T Abs(const T& value)
{
  return value * Sign<T>(value);
}

template<typename T>
Vector2<T> Abs(const Vector2<T>& value)
{
  return { Abs(value.x), Abs(value.y) };
}

template<typename T>
T AbsDiff(const T& a, const T& b)
{
  return a > b ? a - b : b - a;
}

template<typename T>
T Min(const T& a, const T& b)
{
  return a < b ? a : b;
}

template<typename T>
T Min(const Vector2<T>& value)
{
  return Min(value.x, value.y);
}

template<typename T>
T Max(const T& a, const T& b)
{
  return a > b ? a : b;
}

template<typename T, typename T1, typename T2>
T Min(const T1& a, const T2& b)
{
  return a < b ? a : b;
}

template<typename T, typename T1, typename T2>
T Max(const T1& a, const T2& b)
{
  return a > b ? a : b;
}

template<typename T>
T Max(const Vector2<T>& value)
{
  return Max(value.x, value.y);
}

template<typename T, typename T2>
Vector2<T2> Max(const T& a, const Vector2<T2>& b)
{
  return { Max(a, b.x), Max(a, b.y) };
}

template<typename T, typename T2>
Vector2<T2> Max(const Vector2<T2>& b, const T& a)
{
  return { Max(b.x, a), Max(b.y, a) };
}

template<typename T, typename T2>
Vector2<T2> Min(const T& a, const Vector2<T2>& b)
{
  return { Min(a, b.x), Min(a, b.y) };
}

template<typename T, typename T2>
Vector2<T2> Min(const Vector2<T2>& b, const T& a)
{
  return { Min(b.x, a), Min(b.y, a) };
}

template<typename T, typename T2>
Vector2<T> Pow(const Vector2<T>& value, const T2& power)
{
  return { pow(value.x, power), pow(value.y, power) };
}

template<typename T>
T Clamp(const T& value, const T& range_min = 0, const T& range_max = 1)
{
  return Min<T>(Max<T>(value, range_min), range_max);
}

template<typename T>
Vector2<T> Saturate(const Vector2<T>& value)
{
  return { Saturate(value.x), Saturate(value.y) };
}

extern float Floor(const float& value);

extern double Floor(const double& value);

template<typename T>
T Floor(const T& value)
{
  return T(int64_t(value));
}

template<typename T>
Vector2<T> Floor(const Vector2<T>& value)
{
  return { Floor(value.x), Floor(value.y) };
}

extern float Ceil(const float& value);

extern double Ceil(const double& value);

template<typename T>
T Ceil(const T& value)
{
  return T(int64_t(value) + 0.999999);
}

template<typename T>
Vector2<T> Ceil(const Vector2<T>& value)
{
  return { Ceil(value.x), Ceil(value.y) };
}

template<typename T>
T Frac(const T& value)
{
  return value - Floor(value);
}

template<typename T>
T Round(const T& value)
{
  return Floor(value + 0.5);
}

template<typename T = float, typename T2 = float>
inline Vector2<T> Lerp(const Vector2<T>& a, const Vector2<T>& b, const T2& t)
{
  return a * (1.0 - t) + b * t;
}

template<typename T = float, typename T2 = float>
inline T Lerp(const T& a, const T& b, const T2& t)
{
  return a * (1.0 - t) + b * t;
}

