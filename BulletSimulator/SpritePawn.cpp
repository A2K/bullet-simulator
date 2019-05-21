#include "Common.h"

#include "SpritePawn.h"

#include "Draw.h"
#include "Math.h"
#include "World.h"
#include "Math2D.h"
#include "Logger.h"
#include "StringUtils.h"
#include "InputManager.h"
#include "Vector2Stream.h"
#include "BulletManager.h"
#include "ProceduralTexture.h"


SpritePawn::SpritePawn(unsigned int ID, SDL_Renderer * renderer, const int2 & TextureSize, const float2 & Location) :
  Pawn(ID, Location),
  TextureSize(TextureSize)
{
}

void SpritePawn::Move(const float2& direction)
{
  MovementVector += direction;
}

float2 SpritePawn::UpdateVelocity(const float2& delta, double delta_time)
{
  float2 target = {
    isnan(delta.x) ? 0 : delta.x,
    isnan(delta.y) ? 0 : delta.y
  };
  return Velocity = Lerp(Velocity, target, Clamp(Config::Acceleration * delta_time));
}

float2 GetWallsResistance(const float2& Location, const float2 Velocity)
{
  static const double WALL_RESISTANCE_DISTANCE = 6.0f;
  static const double WALL_MAX_RESISTANCE = Config::MovementSpeed;

  float2 total_resistance = 0;

  World::Get().Walls.ForEach([&](const Wall& wall)
  {
    float distance_a = Location.DistanceTo(wall.Ends.A);
    float distance_b = Location.DistanceTo(wall.Ends.B);
    if (Max(distance_a, distance_b) > wall.Ends.Length() && Min(distance_a, distance_b) > WALL_RESISTANCE_DISTANCE)
      return;

    float distance = Math2D::DistanceFromPointToLine(wall.Ends, Location);

    if (isnan(distance)) distance = 0.0f;
    if (distance_a < Abs(distance)) distance = distance_a;
    if (distance_b < Abs(distance)) distance = distance_b;

    if (Abs(distance) < WALL_RESISTANCE_DISTANCE)
    {
      float value = Clamp(1.0 - Abs(distance) / WALL_RESISTANCE_DISTANCE);
      
      float2 direction = -wall.Ends.GetNormal() * Sign(distance);
      float2 normal_velocity = Velocity.Length() * Abs(Velocity.Normalized().Dot(wall.Ends.GetNormal()));
      float2 resistance = direction * value * Min(double(Velocity.Length()), WALL_MAX_RESISTANCE);
      total_resistance += resistance;
    }
  });

  return total_resistance;
}

void SpritePawn::ApplyMovement(double delta_time)
{
  float2 last_location = Location;

  double min_time_to_hit = std::numeric_limits<double>::max();
  float2 min_hit_point;
  float2 min_hit_normal;
  Set<const Wall*> min_hit_walls;
  double time_left = delta_time;

  while (time_left > 0 && Velocity.Length() > 0)
  {
    World::Get().Walls.ForEach([&](const Wall& wall)
    {
      float2 point, normal;
      if (!Math2D::CircleLineIntersection(wall.Ends, Location, Velocity.Normalized(), 5.0f, 100000.0f, point, normal))
        return;

      float2 axis_time = (point - Location) / Velocity;
      if (isnan(axis_time.x)) axis_time.x = INFINITY;
      if (isnan(axis_time.y)) axis_time.y = INFINITY;

      double time_to_hit = (axis_time).Min();

      if (time_to_hit < min_time_to_hit)
      {
        min_hit_walls.clear();
        min_hit_walls += &wall;
        min_time_to_hit = time_to_hit;
        min_hit_point = point;
        min_hit_normal = normal;
      }
      else if (time_to_hit == min_time_to_hit)
      {
        min_hit_walls += &wall;
        min_hit_normal += normal;
      }
    });

    if (min_hit_walls.size() && min_time_to_hit < time_left)
    {
      float2 walls_resistance = GetWallsResistance(Location, Velocity);
      float2 Delta = UpdateVelocity(MovementVector, min_time_to_hit) + walls_resistance;

      min_hit_normal = min_hit_normal.Normalized();

      Location += Delta * min_time_to_hit * Config::MovementSpeed;

      if (min_hit_walls.size() == 1)
        Velocity = Velocity.Reflect(min_hit_normal.Normalized()) * 0.9f;
      else
        Velocity = min_hit_normal.Normalized() * Velocity.Length() * 0.75f;

      time_left -= min_time_to_hit;
    }
    else
    {
      break;
    }

    if (min_time_to_hit == 0) break;
  }

  float2 walls_resistance = GetWallsResistance(Location, Velocity);
  float2 Delta = UpdateVelocity(MovementVector, time_left) + walls_resistance;

  Location += Delta * time_left * Config::MovementSpeed;
  MovementVector = float2(0);
  UpdateFiring(World::Get().CurrentTime + delta_time, last_location);
}

void SpritePawn::Render(SDL_Renderer * renderer, World & world)
{
  static ProceduralTexture sprite(TextureSize, renderer, [](const float2& uv, Color& pixel)
  {
    float value = Clamp(1.0f - uv.DistanceTo(float2(0.5f)) / 0.5f);
    uint8_t V = 0xFF * pow(value, 0.25f);
    pixel.SetRGBA(V, 0, 0xFF - V, 0xFF * value);
  });

  float2 render_offset = world.GetRenderOffset();

  if (Config::ShowViewLine)
  {
    float2 mouse = World::Get().GetManager<InputManager>()->MouseLocation;

    float2 min_hit_point;
    bool hit = false;

    World::Get().Walls.ForEach([&](Wall& wall)
    {
      float2 point;
      if (!Math2D::PointLineIntersection(wall.Ends, Location, Location.DirectionTo(mouse), point))
        return;      
      if (hit && (Location.DistanceTo(point) >= Location.DistanceTo(min_hit_point)))
        return;        
      hit = true;
      min_hit_point = point;
    });

    min_hit_point += Location.DirectionTo(min_hit_point) * 2.0f;

    SDL_SetRenderDrawColor(renderer, Color::RED.WithAlpha(0xFF * 0.5));
    Draw::Line(renderer, Location + render_offset, 
      (hit ? min_hit_point
           : (Location + Location.DirectionTo(mouse) * 100000.0)) + render_offset,
      0.5f);

    if (hit)
    {
      sprite.Render(renderer, min_hit_point + render_offset, float2(0.2), true);
    }
  }

  SDL_SetRenderDrawColor(renderer, Color::WHITE);
  sprite.Render(renderer, Location + render_offset, float2(0.2), true);
}

void SpritePawn::StartFiring(double time, const float2 & target)
{
  Fire.Target = target;
  Fire.LastFireTime = Max(Min(time, Fire.LastFireTime), time - 1.0 / Config::FireRate);
  Fire.Active = true;
}

void SpritePawn::StopFiring(double time)
{
  Fire.Active = false;
}

void SpritePawn::UpdateFiring(double time, const float2 & last_location)
{
  if (!Fire.Active) return;

  Fire.Target = World::Get().GetManager<InputManager>()->MouseLocation;

  int shots = (time - Fire.LastFireTime) / (1.0 / Config::FireRate);

  double delta_time = (1.0 / Config::FireRate) / (time - Fire.LastFireTime);
  for (int i = 0; i < shots; ++i)
  {
    double fire_time = Fire.LastFireTime + (1.0 / Config::FireRate);
    float2 location = Lerp(last_location, Location, Clamp(delta_time));
    World::Get().GetManager<BulletManager>()->Fire(location, location.DirectionTo(Fire.Target), Config::BulletSpeed, fire_time, 0);
    Fire.LastFireTime = fire_time;
  }
}
