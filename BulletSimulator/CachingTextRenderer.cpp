#include "Common.h"

#include "CachingTextRenderer.h"

#include <string>


std::shared_ptr<SDL_Texture> CachingTextRenderer::GetTexture(SDL_Renderer* renderer,
  const std::string& text, const int size, const SDL_Color& color, float2& text_size,
  Text::RenderQuality quality)
{
  Text::TextureCache::Key key = { text, size, color, quality };

  auto texture = Cache.Get(key, text_size);

  if (texture)
  {
    return texture;
  }

  texture = Renderer.RenderTexture(renderer, text.c_str(), size, color, text_size, quality);

  if (texture)
  {
    Cache.Set(key, texture);
  }

  return texture;
}
