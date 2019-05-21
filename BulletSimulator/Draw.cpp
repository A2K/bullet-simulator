#include "Common.h"

#include "Draw.h"

#include "Rect.h"
#include "Math.h"
#include "Point.h"
#include "World.h"
#include "Config.h"
#include "Math2D.h"
#include "LineSegment.h"
#include "TextRenderer.h"
#include "InputManager.h"
#include "WindowManager.h"
#include "Vector2Stream.h"
#include "ProceduralTexture.h"
#include "CachingTextRenderer.h"
#include "ProceduralTextureCache.h"

#include <math.h>


static CachingTextRenderer text_renderer;

size_t Draw::GetTextCacheSize()
{
  return text_renderer.GetPixelCount();
}

double Draw::GetTextCacheHitRate()
{
  return text_renderer.GetHitRate();
}

void Draw::Utility::SetColor(SDL_Renderer* renderer, const Color& color)
{
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

Color Draw::Utility::UpdateTextureTintColor(SDL_Renderer* renderer, SDL_Texture* texture)
{
  Color color;

  SDL_GetRenderDrawColor(renderer, color);

  SDL_SetTextureColorMod(texture, color);

  return color;
}

float2 Draw::Utility::RotatePointRadians(const float2& point, float radians, const float2& center)
{
  const float s = sin(radians);
  const float c = cos(radians);

  const float2 local = point - center;
  
  return {
    center.x + local.x * c - local.y * s,
    center.y + local.x * s + local.y * c
  };
}

float2 Draw::Utility::RotatePointDegrees(const float2& point, float degrees, const float2& center)
{
  return RotatePointRadians(point, DegreesToRadians(degrees), center);
}

void Draw::Point(SDL_Renderer* renderer, const float2& point)
{
  SDL_RenderDrawPoint(renderer, point.x, point.y);
}

void Draw::Line(SDL_Renderer* renderer, const float2& a, const float2& b, float thickness, bool antialias)
{
  if (!antialias)
  {
    SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
    return;
  }


  float2 scale2;
  SDL_RenderGetScale(renderer, &scale2.x, &scale2.y);
  float scale = Max(pow(2.0f, Ceil(log2(scale2.x))), pow(2.0f, Ceil(log2(scale2.y))));
  
  static const float border_scale = 2;
  const float texture_height = Ceil(32 * scale);// pow(2, Ceil(log2(Ceil(32 * Max(1.0f, Config::RenderScale)))));
  const float border_width = Ceil(texture_height / border_scale);

  const int total_texture_height = Ceil(texture_height + border_width * 2);

  static const auto make_generator = [](float power) {
    return [power](const float2& uv, Color& pixel) {
      float value = Max(0.0f, 1.0f - Abs(uv.y - 0.5f) * (4.0f / border_scale * 2.0f));
      pixel.a *= pow(value, power);
    };
  };

  static const auto gen2 = ProceduralTextureCache::Get().AddGenerator("LINE2", 0xFF, make_generator(2));
  static const auto gen4 = ProceduralTextureCache::Get().AddGenerator("LINE4", 0xFF, make_generator(4));
  static const auto gen8 = ProceduralTextureCache::Get().AddGenerator("LINE8", 0xFF, make_generator(8));

  int2 size = { 1, total_texture_height };
  auto sprite = (scale < 2 ? gen2 : (scale < 8 ? gen4 : gen8))(renderer, size);

  float render_height = Ceil(Max(4.0f, 6.0f * thickness / Max(1.0f, scale2.Max())));

  float2 render_scale = float2{ a.DistanceTo(b), render_height } / texture_height;

  float2 render_size = Ceil(render_scale * texture_height);

  float angle_radians = Math2D::GetAngleRadians(a.DirectionTo(b));

  float angle_degrees = Draw::Utility::RadiansToDegrees(angle_radians);

  ::Point center_point = { 0, int(render_size.y * 0.5f) };

  float2 center_point_offset = center_point.As<float2>();

  float2 render_origin = a - center_point_offset;

  ::Rect src = { { 0, border_width }, { texture_height } };
  ::Rect dst = { render_origin, render_size };

  auto screen_to_tex = [&](const float2& xy)
  {
    return Draw::Utility::RotatePointRadians(xy - dst.origin(), -angle_radians, center_point_offset) / render_scale;
  };

  auto tex_to_screen = [&](const float2& xy)
  {
    return Draw::Utility::RotatePointRadians(xy * render_scale, angle_radians, center_point_offset) + dst.origin();
  };

  float2 delta = screen_to_tex(a) - float2(0, texture_height) * 0.5;

  src.origin().y -= delta.y;

  sprite->RenderEx(renderer, dst, src, angle_degrees, &center_point);
}

void Draw::Circle(SDL_Renderer* renderer, const float2& center, float radius, bool fill, float thickness)
{
  if (fill || radius < 6.0f)
  {
    return Draw::CircleFilled(renderer, center, radius);
  }

  static const float texture_size = 256;
  static const float border = 8.0f / texture_size;
  static const float edge_half_width = 6.0f / texture_size * thickness;
  static const float smoothing_width = edge_half_width * 2.0f;
  static const float edge_radius = 1.0f - edge_half_width - border;
  
  static ProceduralTexture sprite(int2(texture_size), 0xFF, renderer,
    [](const float2& uv, Color& pixel)
  {    
    float center_distance = uv.DistanceTo(float2(0.5f)) / 0.5f;

    center_distance = abs(center_distance);

    float cross_distance = Min(Abs(0.5f - uv.x), Abs(0.5f - uv.y)) / 0.5f;

    float alpha = 0;

    if (center_distance < edge_radius + edge_half_width + smoothing_width + border)
    {
      if (center_distance > edge_radius - edge_half_width - smoothing_width)
      {
        float edge_distance = Abs(center_distance - edge_radius);

        if (edge_distance < edge_half_width)
        {
          alpha = 1.0f;
        }
        else
        {
          float value = 1.0f - Clamp(Abs(edge_distance - edge_half_width) / smoothing_width);

          if (center_distance > edge_radius)
          {
            alpha = value;
          }
          else
          {
            alpha = Max(alpha, value);
          }
        }
      }
    }

    pixel.a *= alpha;
  });
  
  Draw::Utility::UpdateTextureTintColor(renderer, sprite.Texture.get());

  sprite.Render(renderer, center, float2(radius / texture_size * 2), true);
}

void Draw::Line(SDL_Renderer* renderer, const LineSegment& segment, float thickness, bool antialias)
{
  Draw::Line(renderer, segment.A, segment.B, thickness, antialias);
}

void Draw::Cross(SDL_Renderer* renderer, const float2& location, float size, float thickness, bool antialias)
{
  Draw::Line(renderer, location - float2(size * 0.5f, 0), location + float2(size * 0.5f, 0), thickness, antialias);

  Draw::Line(renderer, location - float2(0, size * 0.5f), location + float2(0, size * 0.5f), thickness, antialias);
}

void Draw::PointTarget(SDL_Renderer* renderer, const float2& location, float size, float thickness, bool antialias)
{
  Draw::Cross(renderer, location, size * 2, thickness, antialias);

  ::Rect rect = { location - size * 0.5f, size };

  SDL_RenderDrawRect(renderer, &rect);
}

float2 Draw::GetTextSize(const std::string& text, float size)
{
  return text_renderer.GetTextSize(text, size);
}

float2 Draw::Text(SDL_Renderer* renderer, const float2& location, const std::string& text, float size, bool right_align, const Color& color, Text::RenderQuality quality)
{
  float2 text_size;

  float2 scale;
  SDL_RenderGetScale(renderer, &scale.x, &scale.y);
  scale = {
    pow(2.0f, Ceil(log2(scale.x))),
    pow(2.0f, Ceil(log2(scale.y)))
  };

  std::shared_ptr<SDL_Texture> texture = text_renderer.GetTexture(renderer, text, size * scale.x, color, text_size, quality);

  if (!texture) return text_size;
  
  Utility::UpdateTextureTintColor(renderer, texture.get());
  
  ::Rect src = { int2(0), text_size };

  text_size = text_size / scale;

  ::Rect dst = { location, text_size };

  if (right_align) dst.x -= text_size.x;

  SDL_SetTextureBlendMode(texture.get(), quality == Text::SHADED ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

  SDL_RenderCopy(renderer, texture.get(), &src, &dst);

  return text_size;
}

float2 Draw::TextEx(SDL_Renderer* renderer, const float2& location, const std::string& text, float size,
                  double angle, bool center, const float2* center_point, float render_scale, 
                  const Color& color, Text::RenderQuality quality)
{
  float2 text_size;
  std::shared_ptr<SDL_Texture> texture = text_renderer.GetTexture(renderer, text, size * render_scale, color, text_size, quality);
  
  if (!texture) return text_size;
  
  Utility::UpdateTextureTintColor(renderer, texture.get());

  ::Rect src = { int2(0), text_size };

  text_size = Round(text_size / render_scale);

  ::Rect dst = { location - (center ? (text_size * 0.5) : float2(0)), text_size };
    
  SDL_SetTextureBlendMode(texture.get(), quality == Text::SHADED ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

  if (center_point)
  {
    ::Point point = (*center_point) * text_size;
    SDL_RenderCopyEx(renderer, texture.get(), &src, &dst, angle, &point, SDL_FLIP_NONE);
  }
  else
  {
    SDL_RenderCopyEx(renderer, texture.get(), &src, &dst, angle, nullptr, SDL_FLIP_NONE);
  }

  return text_size;
}

float2 Draw::TextEx(SDL_Renderer* renderer, const float2& location, const std::string& text, float size,
            double angle, bool center, const float2& center_point, float render_scale, 
            const Color& color, Text::RenderQuality quality)
{
  return Draw::TextEx(renderer, location, text, size, angle, center, &center_point, render_scale, color, quality);
}

float2 Draw::TextEx(SDL_Renderer* renderer, const float2& location, const std::string& text, float size,
                  double angle, bool center, float render_scale, 
                  const Color& color, Text::RenderQuality quality)
{
  return Draw::TextEx(renderer, location, text, size, angle, center, nullptr, render_scale, color, quality);
}

float2 Draw::TextEx(SDL_Renderer* renderer, const LineSegment& line, const std::string& text, float size,
                    const float2& offset, float render_scale, const Color& color, Text::RenderQuality quality)
{
  size = Min(size, line.Length() * 0.5f);
  
  float angle = Math2D::GetAngleRadians(line);

  static const float HALF_PI = 1.57079632679f;

  if (angle > HALF_PI || angle < -HALF_PI)
  {
    angle += HALF_PI * 2.0f;
  }

  float2 location = (line.A + line.B) * 0.5f + Draw::Utility::RotatePointRadians(offset, angle);

  return Draw::TextEx(renderer, location, text, size, Draw::Utility::RadiansToDegrees(angle), true, render_scale, color, quality);
}

void Draw::CircleFilled(SDL_Renderer* renderer, const float2& center, float radius)
{
  float2 scale;
  SDL_RenderGetScale(renderer, &scale.x, &scale.y);
  scale = {
    pow(2.0f, Ceil(log2(scale.x))),
    pow(2.0f, Ceil(log2(scale.y)))
  };

  float2 texture_size = Ceil(Max(1.0f, scale) * radius) * 32;

  texture_size.x = pow(2, Ceil(log2(texture_size.x)));
  texture_size.y = pow(2, Ceil(log2(texture_size.y)));

  static const float smoothing_width = 0.1f;

  static const auto get_texture = ProceduralTextureCache::Get().AddGenerator("CF", 0xFF, [](const float2& uv, Color& pixel)
  {
    float center_distance = uv.DistanceTo(float2(0.5f)) * 8.0f;
    if (center_distance < 1.0f) return;
    pixel.a *= Clamp(1.0f - (center_distance - 1.0f) / smoothing_width);
  });

  auto sprite = get_texture(renderer, texture_size);

  const float2 texture_scale = float2(radius) / texture_size * 4.0f;
  const float2 render_size = Ceil(texture_scale * texture_size);

  const int2 top_left = center - Ceil(render_size / 2.0f);
  const int2 bottom_right = top_left + render_size;

  const float2 sprite_center = float2(top_left + bottom_right) * 0.5f;

  float2 offset = (sprite_center - center) / texture_scale * 0.5f;

  sprite->Rects.Src = { texture_size / 4.0f + offset, texture_size / 2.0f };
  sprite->Rects.Dst = { top_left, render_size };

  sprite->Render(renderer, sprite->Rects.Dst, sprite->Rects.Src);
}

void Draw::Rect(SDL_Renderer* renderer, const float2& top_left, const float2& size, bool filled)
{
  static ProceduralTexture sprite(int2(1), 0xFF, renderer, {});
  sprite.Rects.Dst = { top_left, size };

  if (filled)
    sprite.Render(renderer);
  else 
    Draw::Rect(renderer, sprite.Rects.Dst, false);
}

void Draw::Rect(SDL_Renderer* renderer, const ::Rect& rect, bool filled)
{
  if (filled)
    return Draw::Rect(renderer, rect.origin(), rect.size(), filled);
  
  SDL_RenderDrawRect(renderer, &rect);
}

float2 Draw::TextBox(SDL_Renderer* renderer,
  const float2& location,
  const List<std::string>& strings, float size,
  const Color& bg_color, const Color& frame_color, const float2& padding,
  bool center, bool draw_above_location,
  int selected_index, const Color& selected_bg_color,
  const Color& color, Text::RenderQuality quality)
{
  Color draw_color;
  SDL_GetRenderDrawColor(renderer, &draw_color.r, &draw_color.g, &draw_color.b, &draw_color.a);

  float2 total_size = { 0, 0 };

  ::Rect selected_rect = { { 0, 0 }, { 0, size } };
  size_t index = 0;
  for (const std::string& str : strings)
  {
    float2 text_size = text_renderer.GetTextSize(str, size);

    if (selected_index >= 0)
    {
      if (index == selected_index)
      {
        selected_rect.y = total_size.y;
      }
    }

    total_size.x = Max(text_size.x, total_size.x);
    total_size.y += text_size.y;

    index++;
  }

  selected_rect.w = total_size.x;
  selected_rect.y += padding.y;
  selected_rect.w += padding.x * 2;
  selected_rect.h += padding.y;
  
  float2 pos = location;

  if (draw_above_location)
  {
    pos.y -= total_size.y + padding.y * 2;
  }

  if (center)
  {
    pos -= total_size * 0.5f + padding;
  }
  
  ::Rect rect = { pos, total_size + padding * 2.0f };

  SDL_SetRenderDrawColor(renderer, bg_color);
  Draw::Rect(renderer, rect.origin(), rect.size());

  if (selected_index >= 0)
  {
    SDL_SetRenderDrawColor(renderer, selected_bg_color);
    selected_rect.origin() += rect.origin();
    Draw::Rect(renderer, selected_rect.origin(), selected_rect.size());
  }  

  SDL_SetRenderDrawColor(renderer, frame_color);
  SDL_RenderDrawRect(renderer, &rect);

  SDL_SetRenderDrawColor(renderer, draw_color);
  
  pos += padding;

  for (const std::string& str : strings)
  {
    pos.y += Draw::Text(renderer, pos, str, size, false, color, quality).y;
  }

  return total_size;
}
