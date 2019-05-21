#pragma once

#include <SDL.h>

#include "Config.h"
#include "TextRenderer.h"
#include "TextTextureCache.h"

#include "RenderQuality.h"

#include <memory>


class CachingTextRenderer
{
public:

  size_t GetPixelCount() const
  {
    return Cache.GetCachedPixelCount();
  }

  double GetHitRate() const
  {
    return Cache.Stats.GetHitRate();
  }

  std::shared_ptr<SDL_Texture> GetTexture(
    SDL_Renderer* renderer,
    const std::string& text,
    const int size,
    const SDL_Color& color, 
    float2& text_size,
    Text::RenderQuality quality = Text::DefaultRenderQuality);

  inline float2 GetTextSize(const std::string& text, float size)
  {
    return Renderer.GetTextSize(text, size);
  }

private:

  Text::Renderer Renderer;
  Text::TextureCache Cache;
};
