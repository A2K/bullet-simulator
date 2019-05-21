#pragma once

#include <SDL.h>

#include "Rect.h"
#include "Types.h"

#include <functional>


class ProceduralTexture
{
public:

  typedef std::function<void(const float2& uv, struct Color& pixel)> Sampler;

public:
  
  int2 Resolution;
    
  struct
  {
    struct
    {
      bool Enabled = false;
      uint8_t Value = 0x0;
    } Initialization;

    struct
    {
      unsigned int PixelFormat = SDL_PIXELFORMAT_ABGR8888;

      SDL_TextureAccess TextureAccess = SDL_TEXTUREACCESS_STATIC;

      SDL_BlendMode BlendMode = SDL_BLENDMODE_BLEND;
    } Texture;
  } Settings;
  
  bool Generated = false;

  std::shared_ptr<SDL_Texture> Texture = nullptr;

  struct
  {
    Rect Src = { { 0, 0 }, { 0, 0 } };
    Rect Dst = { { 0, 0 }, { 0, 0 } };
  } Rects;

  uint8_t BytesPerPixel = 0;

public:

  explicit ProceduralTexture(const int2& dimensions, std::shared_ptr<SDL_Texture> texture);

  explicit ProceduralTexture(const int2& dimensions);

  ProceduralTexture(const int2& dimensions, uint8_t init_value);

  ProceduralTexture(const int2& dimensions, SDL_Renderer* renderer, Sampler generator);

  ProceduralTexture(const int2& dimensions, uint8_t init_value, SDL_Renderer* renderer, Sampler generator);
  
  void Generate(SDL_Renderer* renderer, Sampler color_func);

  void Render(SDL_Renderer* renderer);

  void Render(SDL_Renderer* renderer, const Rect& dst);

  void Render(SDL_Renderer* renderer, const Rect& dst, const Rect& src);

  void Render(SDL_Renderer* renderer, const float2& location, const float2& scale = 1, const bool center = false);  

  void RenderEx(SDL_Renderer* renderer, const float2& location, const float2& scale = 1, const bool center = false,
                float rotation_angle = 0, const struct Point* center_point = nullptr);
  
  void RenderEx(SDL_Renderer* renderer, const Rect& dst, const Rect& src, float rotation_angle, const struct Point* center_point = nullptr);
  
  void RenderEx(SDL_Renderer* renderer, const Rect& dst, const Rect& src, float rotation_angle, const struct Point& center_point);

private:

  void AllocateTextureData(void** ptr, const int2& dimensions);
  void FreeTextureData(void* ptr);

};
