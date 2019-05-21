#include "Common.h"

#include "Color.h"

#include "Math.h"


const Color Color::RED = { 0xFF, 0, 0, 0xFF };

const Color Color::GREEN = { 0, 0xFF, 0, 0xFF };

const Color Color::BLUE = { 0, 0, 0xFF, 0xFF };

const Color Color::WHITE = { 0xFF, 0xFF, 0xFF, 0xFF };

const Color Color::BLACK = { 0, 0, 0, 0xFF };

const Color Color::EMPTY = { 0, 0, 0, 0 };

const Color Color::YELLOW = { 0xFF, 0xFF, 0, 0xFF };

const Color Color::PURPLE = { 0xFF, 0, 0xFF, 0xFF };

const Color Color::CYAN = { 0, 0xFF, 0xFF, 0xFF };

Color Color::Mix(const Color& other, float ratio) const
{
  return Lerp(*this, other, ratio);
}

Color Lerp(const Color& a, const Color& b, float t)
{
  t = Clamp(t);
  return {
    Lerp(a.r, b.r, t),
    Lerp(a.g, b.g, t),
    Lerp(a.b, b.b, t),
    Lerp(a.a, b.a, t)
  };
}
