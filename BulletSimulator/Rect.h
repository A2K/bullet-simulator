#pragma once

#include "Types.h"

#include <SDL.h>


#pragma pack(push, 4)

struct Rect : public SDL_Rect
{
  Rect() :
    SDL_Rect({ 0, 0, 0, 0 })
  {}

  Rect(const int2& origin, const int2& size):
    SDL_Rect({ origin.x, origin.y, size.x, size.y })
  {}

  template<typename T1, typename T2>
  Rect(const Vector2<T1>& origin, const Vector2<T2>& size) :
    SDL_Rect({ int(origin.x), int(origin.y), int(size.x), int(size.y) })
  {}

  int2& origin()
  {
    return *reinterpret_cast<int2*>(&x);
  }

  int2& size()
  {
    return *reinterpret_cast<int2*>(&w);
  }

  const int2& origin() const
  {
    return *reinterpret_cast<const int2*>(&x);
  }

  const int2& size() const
  {
    return *reinterpret_cast<const int2*>(&w);
  }

  float2 center() const
  {
    return { 0.5f * w + x, 0.5f * h + y };
  }
};

#pragma pack(pop)
