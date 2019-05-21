#pragma once

#include "Array.h"
#include "World.h"
#include "Manager.h"


class BulletManager: public Manager
{
public:
  
  void Update(double delta_time) override;

  void Fire(const float2& pos, const float2& dir, float speed, double time, float life_time);

  Set<struct Bullet*> WallAdded(const struct Wall& wall);

  void UpdateBulletCollision(Bullet& bullet, double time = World::Get().CurrentTime);
  
  Bullet::id_t GetNextBulletID();
  Wall::id_t GetNextWallID();

  void UpdateNextBulletID(Bullet::id_t value);
  void UpdateNextWallID(Wall::id_t value);
  struct
  {
    size_t CollisionCount = 0;
    float LastUpdateDeltaTime = 0;
  } Stats;

  uint32_t LastBulletID = 0;

private:

  double GetNearestCollision(const Array<Bullet>& bullets, Bullet const*& hit_bullet);
};

