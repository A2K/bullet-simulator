#include "Common.h"

#include "ProceduralTexture.h"

#include "Draw.h"
#include "Math.h"
#include "Point.h"
#include "World.h"
#include "Config.h"
#include "Logger.h"
#include "Vector2Stream.h"

#include <cmath>
#include <algorithm>
#include <execution>


ProceduralTexture::ProceduralTexture(const int2 & dimensions, std::shared_ptr<SDL_Texture> texture):
  Resolution(dimensions),
  Texture(texture),
  Generated(texture)
{
  Rects.Src.size() = Rects.Dst.size() = Resolution;
}

ProceduralTexture::ProceduralTexture(const int2& dimensions):
  Resolution(dimensions)
{
  Rects.Src.size() = Rects.Dst.size() = Resolution;
}

ProceduralTexture::ProceduralTexture(const int2& dimensions, uint8_t init_value):
  Resolution(dimensions)
{
  Rects.Src.size() = Rects.Dst.size() = Resolution;

  Settings.Initialization.Enabled = true;
  Settings.Initialization.Value = init_value;
}

ProceduralTexture::ProceduralTexture(const int2& dimensions, SDL_Renderer* renderer, Sampler generator):
  Resolution(dimensions)
{
  Rects.Src.size() = Rects.Dst.size() = Resolution;

  Generate(renderer, generator);
}

ProceduralTexture::ProceduralTexture(const int2& dimensions, uint8_t init_value, SDL_Renderer* renderer, Sampler generator) :
  Resolution(dimensions)
{
  Rects.Src.size() = Rects.Dst.size() = Resolution;

  Settings.Initialization.Enabled = true;
  Settings.Initialization.Value = init_value;

  Generate(renderer, generator);
}

void ProceduralTexture::Generate(SDL_Renderer* renderer, Sampler color_func)
{
  if (!BytesPerPixel)
  {
    SDL_PixelFormat* format = SDL_AllocFormat(Settings.Texture.PixelFormat);
    this->BytesPerPixel = format->BytesPerPixel;
    SDL_FreeFormat(format);
  }

  void* data = nullptr;

  AllocateTextureData(&data, Resolution);

  Color* pixels = reinterpret_cast<Color*>(data);
    
  if (color_func)
  {
    if (Config::EnableParallelTextureGeneration)
    {
      Color* end = pixels + Resolution.x * Resolution.y;
      std::for_each(std::execution::par, pixels, end, [&](Color& pixel)
      {
        const size_t index = &pixel - pixels;
        float2 uv = { index, index / Resolution.x };
        uv.x -= Resolution.x * uv.y;
        uv = (uv + 0.5f) / (Resolution);
        color_func(uv, pixel);
      });
    }
    else
    {
      for (int x = 0; x < Resolution.x; ++x)
        for (int y = 0; y < Resolution.y; ++y)
          color_func(
          (float2(x, y) + 0.5f) / Resolution,
            pixels[y * Resolution.x + x]
          );
    }
  }

  if (!Texture)
  {
    Texture = std::shared_ptr<SDL_Texture>(
      SDL_CreateTexture(renderer, Settings.Texture.PixelFormat, Settings.Texture.TextureAccess, Resolution.x, Resolution.y),
      [](SDL_Texture* texture) { SDL_DestroyTexture(texture); });

    SDL_SetTextureBlendMode(Texture.get(), Settings.Texture.BlendMode);
  }

  SDL_UpdateTexture(Texture.get(), &Rects.Src, data, Resolution.x * BytesPerPixel);

  FreeTextureData(data);

  Generated = true;
}

void ProceduralTexture::Render(SDL_Renderer* renderer)
{
  if (!Texture) return;

  Draw::Utility::UpdateTextureTintColor(renderer, Texture.get());

  SDL_RenderCopy(renderer, Texture.get(), &Rects.Src, &Rects.Dst);
}

void ProceduralTexture::Render(SDL_Renderer* renderer, const Rect& dst)
{
  if (!this->Texture) return;

  Draw::Utility::UpdateTextureTintColor(renderer, this->Texture.get());

  SDL_RenderCopy(renderer, Texture.get(), &Rects.Src, &dst);
}

void ProceduralTexture::Render(SDL_Renderer* renderer, const Rect& dst, const Rect& src)
{
  if (!this->Texture) return;

  Draw::Utility::UpdateTextureTintColor(renderer, this->Texture.get());

  SDL_RenderCopy(renderer, Texture.get(), &src, &dst);
}

void ProceduralTexture::Render(SDL_Renderer* renderer, const float2& location, const float2& scale, const bool center)
{
  if (!this->Texture) return;
  
  Rects.Dst.size() = float2(Resolution) * scale;

  float2 target_center = location + float2(Resolution) * scale * (0.5f * (!center));

  Rects.Dst.origin() = location - float2(Resolution) * scale * (0.5f * center);

  Rects.Src.origin() = (Rects.Dst.center() - target_center) / scale;

  Draw::Utility::UpdateTextureTintColor(renderer, this->Texture.get());

  SDL_RenderCopy(renderer, Texture.get(), &Rects.Src, &Rects.Dst);
}

void ProceduralTexture::RenderEx(SDL_Renderer* renderer, const float2& location, const float2& scale, const bool center,
                                float rotation_angle, const Point* center_point)
{
  if (!this->Texture) return;

  Draw::Utility::UpdateTextureTintColor(renderer, this->Texture.get());

  Rects.Dst.origin() = location - (Rects.Dst.size() = Resolution * scale) * (0.5f * center);
  
  SDL_RenderCopyEx(renderer, Texture.get(), &Rects.Src, &Rects.Dst, rotation_angle, center_point, SDL_FLIP_NONE);
}

void ProceduralTexture::RenderEx(SDL_Renderer* renderer, const Rect& dst, const Rect& src, float rotation_angle, const Point* center_point)
{
  if (!this->Texture) return;

  Draw::Utility::UpdateTextureTintColor(renderer, this->Texture.get());

  SDL_RenderCopyEx(renderer, Texture.get(), &src, &dst, rotation_angle, center_point, SDL_FLIP_NONE);
}

void ProceduralTexture::RenderEx(SDL_Renderer * renderer, const Rect & dst, const Rect & src, float rotation_angle, const Point & center_point)
{
  return RenderEx(renderer, dst, src, rotation_angle, &center_point);
}

void ProceduralTexture::AllocateTextureData(void** ptr, const int2& dimensions)
{
  size_t byte_count = dimensions.x * dimensions.y * BytesPerPixel;
  uint8_t* data = new uint8_t[byte_count];
  *ptr = data;
  if (Settings.Initialization.Enabled)
  {
    memset(data, Settings.Initialization.Value, byte_count);
  }
}

void ProceduralTexture::FreeTextureData(void* ptr)
{
  if (ptr)
  {
    uint8_t* data = reinterpret_cast<uint8_t*>(ptr);
    if (data)
    {
      delete[] data;
    }
  }
}
