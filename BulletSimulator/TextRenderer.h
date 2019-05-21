#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

#include "Map.h"
#include "Types.h"
#include "RenderQuality.h"

#include <memory>


namespace Text
{
  class Renderer
  {
  public:

    Renderer();

    ~Renderer();

    static const std::string FontPath;

    int2 GetSize(const char* text, const int size);

    std::shared_ptr<struct SDL_Texture> RenderTexture(
      SDL_Renderer* renderer,
      const char* text,
      const int size,
      const SDL_Color& color,
      float2& text_size,
      Text::RenderQuality quality = Text::DefaultRenderQuality);

    void Render(
      SDL_Renderer* renderer,
      const float2& location,
      const float2& render_offset,
      const char* text,
      int size,
      const SDL_Color& color);

    float2 GetTextSize(const std::string& text, float size);

  private:

    Map<int, TTF_Font*> Fonts;

    TTF_Font* LoadFont(int size);
    TTF_Font* GetFont(int size);

  };
}

