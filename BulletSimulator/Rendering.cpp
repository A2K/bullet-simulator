#include "Common.h"

#include "Rendering.h"

#include "Draw.h"
#include "Math.h"
#include "Color.h"
#include "World.h"
#include "Math2D.h"
#include "SpritePawn.h"
#include "StringUtils.h"
#include "InputManager.h"
#include "BulletManager.h"
#include "WindowManager.h"
#include "WindowManager.h"
#include "ProceduralTexture.h"

#include <SDL.h>


extern Wall wall_template;
extern bool editing_wall;

void RenderBullets(SDL_Renderer* renderer, const float2& render_offset)
{
  World& world = World::Get();

  double time = world.CurrentTime;

  SDL_SetRenderDrawColor(renderer, Color::WHITE);

  world.Bullets.ForEach([renderer, time, render_offset, &world](const Bullet& bullet, size_t index)
  {
    if (bullet.GetLocation(time).DistanceTo(world.RenderCenter) > 1000)
    {
      world.History.ScheduleEvent<EventsHistory::Remove<Bullet>>(time, bullet);
    }
    else
    {
      Draw::CircleFilled(renderer, bullet.GetLocation(time) + render_offset, Config::BulletRadius);
    }
  });
}

void RenderDebugHistory(SDL_Renderer* renderer)
{
  if (!Config::ShowDebugHistory) return;

  World& world = World::Get();

  float2 render_offset = world.GetRenderOffset();

  SpritePawn* pawn = world.GetPawn<SpritePawn>();

  float2 mouse = World::Get().GetManager<InputManager>()->MouseLocation;

  const int2 size = 14;

  const int padding = 4;
  const int hpadding = 4;

  const float max_width = 100;
  int x = world.GetManager<WindowManager>()->RenderResolution.x - max_width;
  int y = padding;

  auto render_event = [&](auto event, uint8_t desaturation) -> int
  {
    std::string type_str = event->Type == EventsHistory::ET_ADD
      ? "+"
      : (event->Type == EventsHistory::ET_REMOVE
        ? "-"
        : "=");

    Color color = event->Type == EventsHistory::ET_ADD
      ? Color::GREEN.WithRB(desaturation)
      : (event->Type == EventsHistory::ET_REMOVE
        ? Color::RED.WithGB(desaturation)
        : Color::YELLOW.WithBlue(desaturation));

    SDL_SetRenderDrawColor(renderer, color.WithAlphaf(0.18f));
    Draw::Rect(renderer, { x - hpadding, y - padding * 0.25f }, { max_width, size.y + padding * 0.5f });

    bool is_bullet = std::dynamic_pointer_cast<EventsHistory::EventData<Bullet>>(event) ||
      std::dynamic_pointer_cast<EventsHistory::EventData<EventsHistory::Update<Bullet>>>(event);

    bool is_collision = false;

    {
      static const char* scales_pos[] = { " s", " m", " h", " d", " w", " y" };
      static const char* scales_neg[] = { " s", "ms", "us", "ns", "ps", "fs" };
      static const double multipliers_pos[] = { 1.0, 60.0, 60.0, 24.0, 7.0, 52.1429 };
      static const double multipliers_neg[] = { 1.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0 };

      auto get_multiplier = [&](int scale)
      {
        return (scale < 0)
          ? multipliers_neg[-(scale + 1)]
          : multipliers_pos[scale];
      };

      auto get_scale_name = [&](int scale)
      {
        return (scale < 0)
          ? scales_neg[-(scale + 1)]
          : scales_pos[scale];
      };

      int scale = 0;

      double t = (event->Time - world.CurrentTime);

      while (Abs(t) < 1.0 && Abs(scale) < 6)
      {
        t *= get_multiplier(--scale);
      }

      while (scale >= 0 && scale < 6 && Abs(t) > get_multiplier(scale + 1))
      {
        t /= get_multiplier(++scale);
      }

      static char buf[256];
      snprintf(buf, 256, "%.2f%s", t, get_scale_name(scale));

      static char buf2[256];
      snprintf(buf2, 256, "%16s", buf);

      static const uint8_t alpha[] = { 255, 225, 205, 185, 165, 145 };

      static const uint8_t glow_neg[] = { 0, 80, 120, 160, 220, 255 };

      float glow_a = Frac(Abs(t) / get_multiplier(scale));
      uint8_t glow = scale > 0 ? 0x0 : Lerp(
        glow_neg[-scale],
        glow_neg[-scale - 1],
        glow_a);

      SDL_SetRenderDrawColor(renderer, Color::WHITE);

      uint8_t a = scale < 0 ? 0xFF : Lerp(
        alpha[Max(0, scale - 1)],
        alpha[scale],
        Frac(Abs(t) / get_multiplier(scale + 1))
      );

      SDL_SetRenderDrawColor(renderer,
        (t > 0 ? Color::GREEN.WithRB(glow) : Color::RED.WithGB(glow))
        .WithAlpha(a));

      Draw::Text(renderer, { x - hpadding * 2, y }, buf2, size.y, true);
    }

    uint32_t id = [&event, &is_bullet, &is_collision]()
    {
      if (is_bullet)
      {
        std::shared_ptr<EventsHistory::EventData<Bullet>> event_bullet =
          std::dynamic_pointer_cast<EventsHistory::EventData<Bullet>>(event);
        if (event_bullet) return event_bullet->Data.ID;

        std::shared_ptr<EventsHistory::EventData<EventsHistory::Update<Bullet>>> event_bullet_update =
          std::dynamic_pointer_cast<EventsHistory::EventData<EventsHistory::Update<Bullet>>>(event);
        if (event_bullet_update) return event_bullet_update->Data.New.ID;
      }
      else
      {
        std::shared_ptr<EventsHistory::EventData<Wall>> event_wall =
          std::dynamic_pointer_cast<EventsHistory::EventData<Wall>>(event);
        if (event_wall) return event_wall->Data.ID;

        std::shared_ptr<EventsHistory::EventData<EventsHistory::Update<Wall>>> event_wall_update =
          std::dynamic_pointer_cast<EventsHistory::EventData<EventsHistory::Update<Wall>>>(event);
        if (event_wall_update) return event_wall_update->Data.New.ID;

        std::shared_ptr<EventsHistory::EventData<EventsHistory::Collision>> event_collision =
          std::dynamic_pointer_cast<EventsHistory::EventData<EventsHistory::Collision>>(event);
        if (event_collision)
        {
          is_collision = true;
          return event_collision->Data.BulletID;
        }
      }

      return uint32_t(0);
    }();

    SDL_SetRenderDrawColor(renderer, color);

    float2 type_text_size = Draw::Text(renderer, { x, y }, type_str, size.y, false, Color::WHITE, Text::SHADED);

    y += (type_text_size.y - size.y) / 2.0f;

    float offset_x = x + size.x + hpadding + type_text_size.x + hpadding;

    SDL_SetRenderDrawColor(renderer, Color::WHITE);
    float2 text_size = Draw::Text(renderer,
      { offset_x, y },
      std::to_string(id), size.y, false, Color::WHITE, Text::SHADED);

    offset_x += text_size.x + hpadding;


    if (is_collision)
    {
      std::shared_ptr<EventsHistory::EventData<EventsHistory::Collision>> event_collision =
        std::dynamic_pointer_cast<EventsHistory::EventData<EventsHistory::Collision>>(event);

      static ProceduralTexture sprite(size, 0xFF, renderer, [](const float2& in_uv, Color& pixel)
      {
        const float2 uv = Abs(in_uv * 2.0f - 1.0f) * 1.5f;
        pixel.a *= pow(Clamp(pow(1.0f - uv.Min(), 2.0f) / uv.Length()), 2.0f);
      });

      float2 location = float2(x + type_text_size.x + hpadding, y) + float2(size) / 2.0f;

      SDL_SetRenderDrawColor(renderer, Color::RED.WithGreen(75));
      sprite.RenderEx(renderer, location, float2(1), true, 22.5);
      sprite.RenderEx(renderer, location, float2(1), true, -22.5);

      location.y -= size.y * 0.5f;
      location.x = x + size.x + type_text_size.x + hpadding * 3 + Max<float>(size.x, text_size.x);

      for (const auto& wall_id : event_collision->Data.WallIDs)
      {
        float2 text_size = Draw::GetTextSize(std::to_string(wall_id), size.y);

        SDL_SetRenderDrawColor(renderer, Color::PURPLE.WithRed(150).WithAlphaf(0.5f));
        Draw::Rect(renderer, location, text_size);

        SDL_SetRenderDrawColor(renderer, Color::PURPLE);
        location.x += Draw::Text(renderer, location, std::to_string(wall_id), size.y, false, Color::WHITE, Text::SHADED).x;
        location.x += hpadding;
      }
    }
    else if (is_bullet)
    {
      SDL_SetRenderDrawColor(renderer, Color::Gray(125));
      float2 center = float2(x + type_text_size.x + hpadding, y) + float2(size) / 2.0f;
      Draw::CircleFilled(renderer, center, float(size.Max()) * 0.5f - 1.0f);

      std::shared_ptr<EventsHistory::EventData<EventsHistory::Update<Bullet>>> event_bullet_update =
        std::dynamic_pointer_cast<EventsHistory::EventData<EventsHistory::Update<Bullet>>>(event);
      if (event_bullet_update)
      {
        float dir_size = Floor(0.5f * size.y) - 0.75f;

        float2 a = center - event_bullet_update->Data.Old.Direction * dir_size;
        float2 b = center + event_bullet_update->Data.New.Direction * dir_size;

        float thickness = 0.75f;
        bool antialias = true;

        SDL_SetRenderDrawColor(renderer, Color::BLACK);
        Draw::Line(renderer, center, a, thickness, antialias);

        SDL_SetRenderDrawColor(renderer, Color::RED);
        Draw::Line(renderer, center, b, thickness, antialias);
      }
    }
    else
    {
      SDL_SetRenderDrawColor(renderer, Color::PURPLE.WithRed(150).WithAlphaf(0.75f));
      Draw::Rect(renderer, { x + size.x / 4.0f + type_text_size.x + hpadding, y }, { size.x / 2.0f, size.y });
    }

    return size.y;
  };

  for (auto iter = world.History.EventsQueue.rbegin(); iter != world.History.EventsQueue.rend(); ++iter)
  {
    if (y >= world.GetManager<WindowManager>()->RenderResolution.y)
      return;

    y += render_event(*iter, 0xFF * 0.25f) + padding;
  }

  for (auto event: world.History.EventsLog)
  {
    if (y >= world.GetManager<WindowManager>()->RenderResolution.y)
      return;

    y += render_event(event, 0xFF * 0.5f) + padding;
  }
}

void RenderDebugOverlay(SDL_Renderer* renderer)
{
  if (!Config::ShowDebugMarkers) return;

  World& world = World::Get();

  float2 render_offset = world.GetRenderOffset();

  float2 mouse = World::Get().GetManager<InputManager>()->MouseLocation;

  SpritePawn* pawn = world.GetPawn<SpritePawn>();

  if (Config::ShowMouseLocation)
  {
    Draw::PointTarget(renderer, mouse + render_offset);
  }

  float2 point;
  float2 normal;
  world.Walls.ForEach([&](Wall& wall)
  {
    if (Config::ShowViewCollisions)
    {
      if (Math2D::CircleLineIntersection(wall.Ends,
        pawn->Location, pawn->Location.DirectionTo(mouse),
        Config::BulletRadius, 10000, point, normal))
      {
        SDL_SetRenderDrawColor(renderer, Color::BLUE);
        Draw::Line(renderer, point + render_offset, point + normal * 25.0f + render_offset, 0.5f);

        float2 reflection = pawn->Location.DirectionTo(mouse).Reflect(normal);
        SDL_SetRenderDrawColor(renderer, Color::YELLOW);
        Draw::Line(renderer, point + render_offset, point + reflection * 25.0f + render_offset, 0.5f);

        SDL_SetRenderDrawColor(renderer, Color::WHITE);
        Draw::CircleFilled(renderer, point + render_offset, Config::BulletRadius);
        Draw::Text(renderer, point + render_offset + float2(12, 8), "wall " + std::to_string(wall.ID), 10);
      }
    }
    
    float angle = Math2D::GetAngleRadians(wall.Ends);

    float2 center_point = { 0.0f, 0.0f };

    static const float HALF_PI = 1.57079632679f;

    static const int font_size = 22;

    float2 offset = 0;

    if (angle > HALF_PI || angle < -HALF_PI)
    {
      angle += HALF_PI * 2.0f;
      offset = { -Draw::GetTextSize(std::to_string(wall.ID), font_size).x, 0 };
      offset = Draw::Utility::RotatePointRadians(offset, angle);
    }

    SDL_SetRenderDrawColor(renderer, Color::WHITE);

    Draw::TextEx(renderer, wall.Ends.A + offset + render_offset, 
      std::to_string(wall.ID), font_size,
      Draw::Utility::RadiansToDegrees(angle), false, &center_point,
      Config::RenderScale * 2.0f, // render scale
      Color::WHITE, Text::BLENDED);
  });

  if (editing_wall)
  {
    SDL_SetRenderDrawColor(renderer, Color::WHITE);

    Draw::TextEx(renderer, wall_template.Ends + render_offset, 
      String::Format(wall_template.Ends.Length(), 2),
      22, float2(0, 12), Config::RenderScale);

    Draw::Text(renderer, wall_template.Ends.A + render_offset,
      std::to_string(int(wall_template.Ends.A.x)) + ':' + std::to_string(int(wall_template.Ends.A.y)), 12);

    Draw::Text(renderer, wall_template.Ends.B + render_offset + float2(0, -20),
      std::to_string(int(wall_template.Ends.B.x)) + ':' + std::to_string(int(wall_template.Ends.B.y)), 12);
  }

  struct BulletTimeCmp
  {
    bool operator()(Bullet::id_t a, Bullet::id_t b) const
    {
      double time = World::Get().CurrentTime;
      auto ba = World::Get().Bullets.Get(a);
      auto bb = World::Get().Bullets.Get(b);
      return ba->Collision.Time == bb->Collision.Time
        ? ba->ID < bb->ID
        : ba->Collision.Time < bb->Collision.Time;
    }
  };

  Map<Wall*, Set<Bullet::id_t, BulletTimeCmp>> wall_count;

  world.Bullets.ForEach([&](Bullet& bullet)
  {
    SDL_SetRenderDrawColor(renderer, Color::WHITE);

    const std::string bullet_id_str = std::to_string(bullet.ID);

    const int font_size = 12;
    
    float2 text_start_pos = bullet.GetLocation(World::Get().CurrentTime) + render_offset;
    text_start_pos.y += Config::BulletRadius;

    float2 text_pos = text_start_pos;
    text_pos.x -= Draw::GetTextSize(bullet_id_str, font_size).x * 0.5f;

    Draw::Text(renderer, text_pos, bullet_id_str, font_size);

    if (!bullet.Collision.Hits) return;

    SDL_SetRenderDrawColor(renderer, Color::GREEN);
    Draw::Line(renderer, bullet.GetLocation(World::Get().CurrentTime) + render_offset, bullet.Collision.Location + render_offset, 0.5f);

    SDL_SetRenderDrawColor(renderer, Color::BLUE.WithAlpha(0xFA));
    Draw::Line(renderer, bullet.Collision.Location + render_offset,
      bullet.Collision.Location + bullet.Collision.Normal * Max(Config::BulletRadius, 15.0) + render_offset, 0.5f);
    SDL_SetRenderDrawColor(renderer, Color::YELLOW.WithAlpha(0xFA));
    Draw::Line(renderer, bullet.Collision.Location + render_offset,
      bullet.Collision.Location + bullet.Collision.Direction * Max(Config::BulletRadius, 15.0) + render_offset, 0.5f);

    SDL_SetRenderDrawColor(renderer, Color::RED.WithAlpha(0xEA));
    Draw::CircleFilled(renderer, bullet.Collision.Location + render_offset, Config::BulletRadius);

    const std::string wall_ids_str = String::Join(bullet.Collision.WallIDs, ",");

    const float2 text_size = Draw::GetTextSize(wall_ids_str, font_size);

    text_pos = text_start_pos;
    text_pos.y += text_size.y;
    text_pos.x -= text_size.x * 0.5f;

    SDL_SetRenderDrawColor(renderer, Color::PURPLE);

    Draw::Text(renderer, text_pos, wall_ids_str, font_size);

    for (auto wall_id : bullet.Collision.WallIDs)
    {
      Wall* wall = World::Get().Walls.Get(wall_id);
      if (wall) wall_count[wall] += bullet.ID;
    }
  });

  for(const auto& pair: wall_count)
  {
    const auto& wall = pair.first;
    const auto& bullet_ids = pair.second;

    SDL_SetRenderDrawColor(renderer, Color::RED);
    Draw::Line(renderer, wall->Ends + render_offset, 0.5f);

    SDL_SetRenderDrawColor(renderer, Color::PURPLE);
    Draw::Text(renderer, wall->Ends.B + render_offset, String::Join(bullet_ids, ","), 12);
  }
}

void RenderWalls(SDL_Renderer* renderer, const float2& render_offset)
{
  World& world = World::Get();

  static const float thickness = 0.75f;

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  SDL_SetRenderDrawColor(renderer, Color::PURPLE);
  world.Walls.ForEach([&](Wall& wall)
  {
    Draw::Line(renderer, wall.Ends + render_offset, thickness);
  });

  if (editing_wall)
  {
    SDL_SetRenderDrawColor(renderer, Color::CYAN);
    Draw::Line(renderer, wall_template.Ends + render_offset, thickness);
  }
}