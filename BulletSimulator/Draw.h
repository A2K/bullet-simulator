#pragma once

#include "Rect.h"
#include "Color.h"
#include "Types.h"
#include "RenderQuality.h"

#include <SDL.h>

#include <string>


struct LineSegment;
template<typename T> class List;

static inline int SDL_SetRenderDrawColor(SDL_Renderer* renderer, const Color& color)
{
  return SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

static inline int SDL_GetRenderDrawColor(SDL_Renderer* renderer, Color& color)
{
  return SDL_GetRenderDrawColor(renderer, &color.r, &color.g, &color.b, &color.a);
}

static inline int SDL_SetTextureColorMod(SDL_Texture* texture, const Color& color)
{
  int code = SDL_SetTextureAlphaMod(texture, color.a);
  if (code != 0) return code;
  return SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
}

static inline int SDL_GetTextureColorMod(SDL_Texture* texture, Color& color)
{
  int code = SDL_GetTextureAlphaMod(texture, &color.a);
  if (code != 0) return code;
  return SDL_GetTextureColorMod(texture, &color.r, &color.g, &color.b);
}

namespace Draw
{
  namespace Utility
  {
    extern void SetColor(SDL_Renderer* renderer, const Color& color);

    extern Color UpdateTextureTintColor(SDL_Renderer* renderer, SDL_Texture* texture);

    extern float2 RotatePointRadians(const float2& point, float radians, const float2& center = float2(0, 0));

    extern float2 RotatePointDegrees(const float2& point, float degrees, const float2& center = float2(0, 0));

    static const float DegreesInRadian = 180.0f / 3.141592653589793238463f;

    template<typename T = float>
    T DegreesToRadians(const T& degrees) { return degrees / DegreesInRadian; }

    template<typename T = float>
    T RadiansToDegrees(const T& radians) { return radians * DegreesInRadian; }

    class ColorScope
    {
      SDL_Renderer* Renderer;
      Color OriginalColor;

    public:

      ColorScope(SDL_Renderer* renderer, const Color& color) :
        Renderer(renderer)
      {
        if (Renderer)
        {
          SDL_GetRenderDrawColor(Renderer, OriginalColor);
          SDL_SetRenderDrawColor(Renderer, color);
        }
      }

      virtual ~ColorScope()
      {
        if (Renderer)
        {
          SDL_SetRenderDrawColor(Renderer, OriginalColor);
        }
      }
    };
  }

  extern void Point(SDL_Renderer* renderer, const float2& point);

  extern void Line(SDL_Renderer* renderer, const float2& a, const float2& b, float thickness = 1.0f, bool antialias = true);
  
  extern void Circle(SDL_Renderer* renderer, const float2& center, float radius, bool fill = false, float thickness = 1.0f);

  extern void CircleFilled(SDL_Renderer* renderer, const float2& center, float radius);

  extern void Line(SDL_Renderer* renderer, const LineSegment& segment, float thickness = 1.0f, bool antialias = true);

  extern void Cross(SDL_Renderer* renderer, const float2& location, float size = 16, float thickness = 1.0f, bool antialias = true);

  extern void Rect(SDL_Renderer* renderer, const ::Rect& rect, bool filled = true);

  extern void Rect(SDL_Renderer* renderer, const float2& top_left, const float2& size, bool filled = true);

  extern void PointTarget(SDL_Renderer* renderer, const float2& location, float size = 16, float thickness = 1.0f, bool antialias = true);

  extern float2 GetTextSize(const std::string& text, float size);

  extern float2 Text(SDL_Renderer* renderer, const float2& location, const std::string& text, float size = 16.0f, bool right_align = false, const Color& color = Color::WHITE, Text::RenderQuality quality = Text::SHADED);

  extern size_t GetTextCacheSize();

  extern double GetTextCacheHitRate();

  extern float2 TextEx(SDL_Renderer* renderer, const float2& location, const std::string& text, float size = 16.0f,
    double angle = 0.0, bool center = false, const float2* center_point = nullptr, float render_scale = 1.0f, 
    const Color& color = Color::WHITE, Text::RenderQuality quality = Text::DefaultRenderQuality);

  extern float2 TextEx(SDL_Renderer* renderer, const float2& location, const std::string& text, float size = 16.0f,
    double angle = 0.0, bool center = false, const float2& center_point = float2(0.5f, 0.5f), float render_scale = 1.0f, 
    const Color& color = Color::WHITE, Text::RenderQuality quality = Text::DefaultRenderQuality);

  extern float2 TextEx(SDL_Renderer* renderer, const float2& location, const std::string& text, float size = 16.0f,
    double angle = 0.0, bool center = false, float render_scale = 1.0f, 
    const Color& color = Color::WHITE, Text::RenderQuality quality = Text::DefaultRenderQuality);

  extern float2 TextEx(SDL_Renderer* renderer, const LineSegment& line, const std::string& text, float size = 16.0f,
    const float2& offset = float2(0.0f), float render_scale = 1.0f,
    const Color& color = Color::WHITE, Text::RenderQuality quality = Text::DefaultRenderQuality);

  extern float2 TextBox(SDL_Renderer* renderer, const float2& location, const List<std::string>& strings, float size = 16.0f,
    const Color& bg_color = Color::Gray(75), const Color& frame_color = Color::WHITE, const float2& padding = float2(4, 4),
    bool center = false, bool draw_above_location = false,
    int selected_index = -1, const Color& selected_bg_color = Color::Gray(150),
    const Color& color = Color::WHITE, Text::RenderQuality quality = Text::SHADED);
}
