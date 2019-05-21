#include "Common.h"

#include "BulletManager.h"

#include "Config.h"
#include "Logger.h"
#include "StringUtils.h"
#include "Vector2Stream.h"
#include "WindowManager.h"
#include "NetworkManager.h"

#include <mutex>
#include <iterator>
#include <algorithm>
#include <execution>


static struct {
  std::mutex BulletID;
  std::mutex WallID;
} Mutexes;

struct Collision
{
  double Time = std::numeric_limits<double>::infinity();
  uint32_t WallID = 0;
  float2 Location;
  float2 Normal;

  bool operator<(const Collision& other) const
  {
    return (this->Time == other.Time)
      ? this->WallID < other.WallID
      : this->Time < other.Time;
  }
};

float2 GetReflectionNormal(const Set<Collision>& collisions)
{
  if (collisions.size() == 1) return collisions.First().Normal;
  float2 sum = 0;
  for (const Collision& collision : collisions)
  {
    if (Config::LogCollisions)
      LOG << "wall[" << collision.WallID << "] hit normal: " << collision.Normal;
    sum += collision.Normal;
  }
  return sum;
};

void BulletManager::UpdateBulletCollision(Bullet& bullet, double time)
{
  if (Config::LogCollisions)
    LOG << "UpdateBulletNextHit: bullet[" << bullet.ID << "] at " << time << " last wall ids: [" << String::Join(bullet.Collision.WallIDs, ", ") << "]";

  Collision current;
  Set<Collision> collisions;

  World::Get().Walls.ForEach([&bullet, time, &collisions](const Wall& wall)
  {
    Collision current;
    current.Time = bullet.IntersectWall(wall, current.Location, current.Normal, time);

    if (bullet.Collision.WallIDs.Contains(wall.ID)) return;
    if (isinf(current.Time)) return;
    current.WallID = wall.ID;
    collisions += current;
  });
  
  if (!collisions.size())
  {
    bullet.Collision.Hits = false;
    return;
  }

  Collision min_collision = collisions.First();
  double min_collision_time = min_collision.Time;

  Set<Wall::id_t> min_wall_ids;
  Set<Collision> min_collisions;
  for(auto& collision: collisions)
  {
    if (collision.Time == min_collision_time)
    {
      min_collisions += collision;
      min_wall_ids += collision.WallID;
    }
  }

  if (Config::LogCollisions)
    LOG << "UpdateBulletNextHit: min_wall_ids: [" << String::Join(min_wall_ids, ", ") << "] at " << min_collision_time;

  if (!min_wall_ids.size())
  {
    bullet.Collision.Hits = false;
    return;
  }

  if (Abs(bullet.Collision.Time - min_collision_time) > 0.0001)
  {
    bullet.Collision.WallIDs.clear();
    bullet.Collision.Normal = 0;
  }
  else
  {
    if (bullet.Collision.WallIDs.size())
    {
      bullet.Direction = bullet.Direction.Reflect(bullet.Collision.Normal.Normalized());
    }
  }
  
  bullet.Collision.Hits = true;

  bullet.Collision.Time = min_collision.Time;
  bullet.Collision.Location = min_collision.Location; 

  bullet.Collision.WallIDs += min_wall_ids;
  
  bullet.Collision.Normal += GetReflectionNormal(min_collisions);  

  if (bullet.Collision.WallIDs.size() == 1)
  {
    bullet.Collision.Direction = bullet.Direction.Reflect(bullet.Collision.Normal);
  }
  else
  {
    bullet.Collision.Direction = bullet.Collision.Normal.Normalized();
  }
}

double BulletManager::GetNearestCollision(const Array<Bullet>& bullets, Bullet const*& hit_bullet)
{
  const double time = World::Get().CurrentTime;

  double min_collision_time = std::numeric_limits<double>::infinity();

  for (const Bullet& bullet : bullets)
  {
    if (isinf(bullet.Collision.Time)) continue;

    if (bullet.Collision.Time < min_collision_time)
    {
      min_collision_time = bullet.Collision.Time;
      hit_bullet = &bullet;
    }
  }

  return min_collision_time;
}

void BulletManager::Update(double delta_time)
{
  World& world = World::Get();

  //world.Simulate(world.CurrentTime + delta_time);
}

Bullet::id_t BulletManager::GetNextBulletID()
{
  std::lock_guard<std::mutex> lock{ Mutexes.BulletID };

  if (++Config::LastBulletId == 0)
    ++Config::LastBulletId;

  return Config::LastBulletId;
}

Wall::id_t BulletManager::GetNextWallID()
{
  std::lock_guard<std::mutex> lock{ Mutexes.WallID };

  if (++Config::LastWallId == 0)
    ++Config::LastWallId;

  return Config::LastWallId;
}

void BulletManager::UpdateNextBulletID(Bullet::id_t value)
{
  std::lock_guard<std::mutex> lock{ Mutexes.BulletID };
  Config::LastBulletId = Max(Config::LastBulletId, value);
}

void BulletManager::UpdateNextWallID(Wall::id_t value)
{
  std::lock_guard<std::mutex> lock{ Mutexes.WallID };
  Config::LastWallId = Max(Config::LastWallId, value);
}

void BulletManager::Fire(const float2& pos, const float2& dir, float speed, double time, float life_time)
{
  World& world = World::Get();

  Bullet bullet(GetNextBulletID(), pos, dir, speed, time, life_time);  

  world.History.ScheduleEvent<EventsHistory::Add<Bullet>>(time, bullet, false);

  world.GetManager<NetworkManager>()->Fire(bullet);
}

Set<Bullet*> BulletManager::WallAdded(const Wall& wall)
{
  World& world = World::Get();

  double time;
  float2 intersection;
  float2 normal;

  Set<Bullet*> updated_bullets;
  world.Bullets.ForEach([&](Bullet& bullet)
  {
    time = bullet.IntersectWall(wall, intersection, normal, world.CurrentTime);

    if (isinf(time)) return;

    if (bullet.Collision.Hits && bullet.Collision.WallIDs.size() && time > bullet.Collision.Time) return;

    if (time < wall.Time.GetTime(world.CurrentTime)) return;

    updated_bullets += &bullet;

    bullet.Collision.Hits = true;

    if (!bullet.Collision.Hits || time < bullet.Collision.Time || !bullet.Collision.WallIDs.size())
    {
      bullet.Collision.WallIDs.clear();
      bullet.Collision.Normal = normal;
    } 
    else if (bullet.Collision.WallIDs.size())
    {
      bullet.Collision.Normal += normal;
    }

    bullet.Collision.WallIDs += wall.ID;
    bullet.Collision.Time = time;
    bullet.Collision.Location = intersection;
    bullet.Collision.Direction = bullet.Direction.Reflect(bullet.Collision.Normal.Normalized());
  });

  return updated_bullets;
}

