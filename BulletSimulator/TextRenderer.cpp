#include "Common.h"

#include "TextRenderer.h"

#include "Rect.h"
#include "Color.h"
#include "Config.h"
#include "Logger.h"

#include <filesystem>


using namespace Text;

const std::string Renderer::FontPath = "Inconsolata-Regular.ttf";

Renderer::Renderer()
{
  static bool TTF_INITIALIZED = false;

  if (!TTF_INITIALIZED)
  {
    if (TTF_Init() < 0) 
    {
      LOG << "Couldn't initialize SDL_ttf: " << TTF_GetError();
    }
    else
    {
      TTF_INITIALIZED = true;
    }
  }
}

TTF_Font* Renderer::LoadFont(int size)
{
  TTF_Font* font = nullptr;

  auto path_abs = std::filesystem::absolute(std::filesystem::path(FontPath));  
  if (!std::filesystem::exists(path_abs))
  {
    path_abs = std::filesystem::absolute(std::filesystem::path("Fonts/" + FontPath));
    if (!std::filesystem::exists(path_abs))
    {
      path_abs = std::filesystem::absolute(std::filesystem::path("../Fonts/" + FontPath));
      if (!std::filesystem::exists(path_abs))
      {
        path_abs = std::filesystem::absolute(std::filesystem::path("../../Fonts/" + FontPath));
        if (!std::filesystem::exists(path_abs))
        {
          path_abs = std::filesystem::absolute(std::filesystem::path(Config::BinaryPath + "/" + FontPath));
          if (!std::filesystem::exists(path_abs))
          {
            path_abs = std::filesystem::absolute(std::filesystem::path(Config::BinaryPath + "/Fonts/" + FontPath));
          }
        }
      }
    }
  }

  if (!(font = TTF_OpenFont(path_abs.string().c_str(), size)))
  {
    LOG_COUT << "Failed to open font " << FontPath << ": " << TTF_GetError();
  }

  return font;
}

TTF_Font* Renderer::GetFont(int size)
{
  auto iter = Fonts.find(size);

  if (iter != Fonts.end())
  {
    return iter->second;
  }

  return Fonts[size] = LoadFont(size);
}

int2 Renderer::GetSize(const char* text, const int size)
{
  int2 text_size;
  TTF_SizeText(GetFont(size), text, &text_size.x, &text_size.y);
  return text_size;
}

std::shared_ptr<SDL_Texture> Renderer::RenderTexture(SDL_Renderer* renderer, const char* text, const int size, const SDL_Color& color, float2& text_size, RenderQuality quality)
{
  if (strlen(text) == 0) return nullptr;

  SDL_Surface* surface = nullptr;

  if (quality == SOLID)
  {
    surface = TTF_RenderText_Solid(GetFont(size), text, color);
  }
  else if (quality == SHADED) 
  {
    surface = TTF_RenderText_Shaded(GetFont(size), text, color, Color::EMPTY);
  }
  else if (quality == BLENDED) 
  {
    surface = TTF_RenderText_Blended(GetFont(size), text, color);
  }

  if (!surface) return nullptr;

  text_size.x = float(surface->w);
  text_size.y = float(surface->h);

  std::shared_ptr<SDL_Texture> texture = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer, surface), SDL_DestroyTexture);

  SDL_FreeSurface(surface);

  SDL_SetTextureBlendMode(texture.get(), quality == SHADED ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

  return texture;
}

void Renderer::Render(SDL_Renderer* renderer, const float2& location, const float2& render_offset, const char* text, int size, const struct SDL_Color& color)
{
  float2 text_size;
  std::shared_ptr<SDL_Texture> texture = RenderTexture(renderer, text, size, color, text_size);

  Rect rect = { render_offset, text_size };

  SDL_RenderCopy(renderer, texture.get(), NULL, &rect);
}

Renderer::~Renderer()
{
  for (auto pair : Fonts)
  {
    TTF_CloseFont(pair.second);
  }
}

float2 Renderer::GetTextSize(const std::string& text, float size)
{
  int w, h;
  TTF_SizeText(GetFont(size), text.c_str(), &w, &h);
  return float2(w, h);
}
