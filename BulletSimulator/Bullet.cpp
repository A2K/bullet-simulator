#include "Bullet.h"

#include "Common.h"

#include "Wall.h"
#include "Math.h"
#include "World.h"
#include "Logger.h"
#include "Config.h"
#include "Math2D.h"
#include "EventsHistory.h"
#include "StringUtils.h"
#include "Vector2Stream.h"


Set<Wall::id_t> Bullet::ApplyCollision()
{
  if (!Collision.Hits || !Collision.WallIDs.size() || Collision.Time > World::Time())
    return {};

  World& world = World::Get();

  Set<Wall*> hit_walls;

  for (auto wall_id : Collision.WallIDs)
  {
    Wall* wall = world.Walls.Get(wall_id);
    if (wall) hit_walls += wall;
  }

  if (hit_walls.size() < Collision.WallIDs.size())
  {
    float2 point, normal, normal_sum = 0;

    bool hits = false;

    Collision.WallIDs.clear();

    for (Wall* wall : hit_walls)
    {
      double hit_time = this->IntersectWall(*wall, point, normal, World::Time());
      if (hit_time != Collision.Time) continue;
      hits = true;
      normal_sum += normal;
      Collision.WallIDs += wall->ID;
    }

    if (!hits)
    {
      Collision.Hits = false;
      return {};
    }
  }

  if (Config::LogCollisions)
  {
    LOG
      << "bullet[" << ID << "] hit "
      << (Collision.WallIDs.size() > 1 ? "walls" : "wall")
      << " " << String::Join(Collision.WallIDs)
      << " at " << World::Time()
      << " normal: " << Collision.Normal.Normalized()
      << " direction: " << Direction << " -> " << Collision.Direction;
  }

  Time = Collision.Time;
  Location = Collision.Location;
  Direction = Collision.Direction;

  Collision.Hits = false;

  return Collision.WallIDs;
}

Bullet::Bullet(const Bullet * data) :
  Bullet(*data)
{}

Bullet::Bullet(const id_t ID, const float2 & location, const float2 & direction, const float speed, const double time, const float lifetime) :
  Time(time), ID(ID), Location(location), Direction(direction), Speed(speed), Lifetime(lifetime)
{
}

float2 Bullet::GetLocation(double time) const
{
  return Location + Direction * Speed * (time - Time);
}

double Bullet::IntersectWall(const Wall& wall, float2& point, float2& normal, double time) const
{
  float2 location = Location + Direction * Speed * Max(0.0, time - Time);
  if (isnan(location.x))
  {
    LOG << "Bullet::IntersectWall location.x is not a number";
  }
  if (Math2D::CircleLineIntersection(wall.Ends, location, Direction, Config::BulletRadius, 10000.0f, point, normal))
  {
    if (isnan(point.x))
    {
      LOG << "Bullet::IntersectWall point.x is not a number";
    }
    return time + location.DistanceTo(point) / Speed; // seconds
  }

  return std::numeric_limits<double>::infinity();
}

std::ostream& operator<<(std::ostream& stream, const Bullet& bullet)
{
  stream
    << "Bullet[ID=" << bullet.ID
    << ",Location=" << bullet.Location
    << ",Direction=" << bullet.Direction
    << ",NextHit[Time=" << bullet.Collision.Time
    << ",WallIDs=[" << String::Join(bullet.Collision.WallIDs) << "]"
    << ",Location=" << bullet.Collision.Location
    << ",Normal=" << bullet.Collision.Direction
    << "]]";
  return stream;
}
