#pragma once

#include <SDL.h>

#include "Vector2.h"

#include <cstdint>


#pragma pack(push, 4)

struct Color: public SDL_Color
{
  Color() :
    SDL_Color({ 0, 0, 0, 0 })
  {}
  
  explicit Color(uint8_t C) :
    SDL_Color({ C, C, C, C })
  {}

  explicit Color(uint32_t C):
    SDL_Color({ (C >> 24) & 0xFF, (C >> 16) & 0xFF, (C >> 8) & 0xFF, C & 0xFF })
  {}

  Color(uint8_t R, uint8_t G, uint8_t B) :
    SDL_Color({ R, G, B, 0xFF })
  {}

  Color(uint8_t R, uint8_t G, uint8_t B, uint8_t A) :
    SDL_Color({ R, G, B, A })
  {}

  void SetRGB(uint8_t L)
  {
    r = g = b = L;
  }

  void SetRGB(uint8_t R, uint8_t G, uint8_t B)
  {
    r = R; g = G; b = B;
  }

  void SetRGBHex(uint32_t C)
  {
    r = (C >> 16) & 0xFF;
    g = (C >> 8) & 0xFF;
    b = C & 0xFF;
  }

  void SetRGBA(uint8_t L)
  {
    r = g = b = a = L;
  }

  void SetRGBA(uint8_t R, uint8_t G, uint8_t B, uint8_t A)
  {
    r = R; g = G; b = B; a = A;
  }

  void SetRGBAHex(uint32_t C)
  {
    *this = { (C >> 24) & 0xFF, (C >> 16) & 0xFF, (C >> 8) & 0xFF, C & 0xFF };
  }

  void SetRGB_A(uint8_t L, uint8_t A)
  {
    r = g = b = L; a = A;
  }

  template<typename T>
  void SetRGBA(const T& R, const T& G, const T& B, const T& A)
  {
    r = R; g = G; b = B; a = A;
  }

  Color WithRed(uint8_t value) const
  {
    return { value, g, b, a };
  }

  Color WithGreen(uint8_t value) const
  {
    return { r, value, b, a};
  }

  Color WithBlue(uint8_t value) const
  {
    return { r, g, value, a};
  }
  
  Color WithRG(uint8_t value) const
  {
    return { value, value, b, a };
  }

  Color WithGB(uint8_t value) const
  {
    return { r, value, value, a };
  }

  Color WithRB(uint8_t value) const
  {
    return { value, g, value, a };
  }

  Color WithRGB(uint8_t value) const
  {
    return { value, value, value, a };
  }
  
  Color WithAlpha(uint8_t value) const
  {
    return { r, g, b, value };
  }

  Color WithAlphaf(float value) const
  {
    return { r, g, b, uint8_t(value * 0xFF) };
  }

  static const struct Color RED;
  static const struct Color GREEN;
  static const struct Color BLUE;
  static const struct Color WHITE;
  static const struct Color BLACK;
  static const struct Color EMPTY;
  static const struct Color YELLOW;
  static const struct Color PURPLE;
  static const struct Color CYAN;

  static inline Color Gray(uint8_t brightness, uint8_t alpha = 0xFF)
  {
    return { brightness, brightness, brightness, alpha };
  }

  operator SDL_Color&()
  {
    return *this;
  }

  Color& R(uint8_t value)
  {
    r = value;
    return *this;
  }

  Color& G(uint8_t value)
  {
    g = value;
    return *this;
  }

  Color& B(uint8_t value)
  {
    b = value;
    return *this;
  }

  Color& A(uint8_t value)
  {
    a = value;
    return *this;
  }
  
  template<size_t channel1>
  uint8_t Get() const
  {
    uint8_t* channels = (uint8_t*)this;
    return channels[channel1];
  }

  template<size_t channel1, size_t channel2>
  Vector2<uint8_t> Get() const
  {
    uint8_t* channels = (uint8_t*)this;
    return { channels[channel1], channels[channel2] };
  }

  Color& Set(uint8_t v)
  {
    return *this = { v, v, v, v };
  }

  template<size_t channel>
  Color& Set(uint8_t value)
  {
    uint8_t* channels = (uint8_t*)this;
    channels[channel] = value;
    return *this;
  }

  template<size_t channel1, size_t channel2>
  Color& Set(uint8_t value)
  {
    uint8_t* channels = (uint8_t*)this;
    channels[channel1] = channels[channel2] = value;    
    return *this;
  }

  template<size_t channel1, size_t channel2, size_t channel3>
  Color& Set(uint8_t value)
  {
    uint8_t* channels = (uint8_t*)this;

    channels[channel1] = 
      channels[channel2] = 
      channels[channel3] = value;

    return *this;
  }

  Color Mix(const Color& other, float ratio = 0.5f) const;

};

#pragma pack(pop)

extern Color Lerp(const Color& a, const Color& b, float t);
