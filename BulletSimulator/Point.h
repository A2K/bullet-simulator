#pragma once

#include "Types.h"

#include <SDL.h>


#pragma pack(push, 4)

struct Point : SDL_Point
{  
  Point(): 
    SDL_Point({ 0, 0 })
  {}

  template<typename T>
  Point(const T& x, const T& y) :
    SDL_Point({ int(x), int(y) })
  {}

  template<typename T>
  Point(const Vector2<T>& v) :
    SDL_Point({ int(v.x), int(v.y) })
  {}

  Point& operator=(const int2& v) 
  {
    x = v.x;
    y = v.y;
    return *this;
  }

  operator int2&() 
  {
    return *reinterpret_cast<int2*>(this);
  }

  template<typename T2>
  explicit operator Vector2<T2>() const
  {
    return { x , y };
  }

  template<typename T2>
  T2 As() const
  {
    return { x, y };
  }
};

#pragma pack(pop)
