#pragma once

#include "Types.h"


#pragma pack(push, 4)

struct LineSegment
{
  float2 A;
  float2 B;

  LineSegment()
  {}

  LineSegment(const float2& A, const float2& B) :
    A(A), B(B)
  {}

  float2 GetDirection() const;

  float2 GetNormal() const;

  float Length() const
  {
    return A.DistanceTo(B);
  }

  float2 Reflect(const float2& direction) const;

  bool Intersect(const LineSegment& other, float2* point = nullptr) const;

  bool Intersect(const LineSegment& other, float2& point) const;

  template<typename Value>
  LineSegment& operator+=(const Value& value)
  {
    A += value;
    B += value;
    return *this;
  }

  LineSegment& operator+=(const LineSegment& value)
  {
    A += value.A;
    B += value.B;
    return *this;
  }

  LineSegment operator+(const LineSegment& value) const
  {
    return { A + value.A, B + value.B };
  }

  template<typename Value>
  LineSegment operator+(const Value& value) const
  {
    return { A + value, B + value };
  }

  template<typename Value>
  LineSegment operator-(const Value& value) const
  {
    return { A - value, B - value };
  }

  LineSegment operator-(const LineSegment& value) const
  {
    return { A - value.A, B - value.B };
  }

  template<typename Value>
  LineSegment operator*(const Value& value) const
  {
    return { A * value, B * value };
  }

  template<typename Value>
  LineSegment& operator*=(const Value& value)
  {
    A *= value;
    B *= value;
    return *this;
  }

  template<typename Value>
  LineSegment& operator/=(const Value& value)
  {
    A *= value;
    B *= value;
    return *this;
  }

  template<typename Value>
  LineSegment& operator-=(const Value& value)
  {
    A -= value;
    B -= value;
    return *this;
  }

};

#pragma pack(pop)
