#pragma once

#include "Set.h"
#include "Wall.h"
#include "Types.h"
#include "Vector2.h"
#include "Timestamp.h"
#include "ProtocolEnums.h"


#pragma pack(push, 4)

struct Bullet
{
public:

  typedef uint32_t id_t;
  
  double Time;
  id_t ID = 0;

  float2 Location;
  float2 Direction;

  float Speed = 0;
  float Lifetime = 0;

  struct {
    bool Hits = false;
    double Time;
    Set<Wall::id_t> WallIDs;
    float2 Location;
    float2 Direction;
    float2 Normal;
  } Collision;
  
public:

  Bullet() {};

  explicit Bullet(const Bullet* data);

  Bullet(const id_t ID, const float2& location, const float2& direction, const float speed, const double time, const float lifetime);

  Set<Wall::id_t> ApplyCollision();

  float2 GetLocation(double time) const;

  // returns intersection time
  double IntersectWall(const struct Wall& wall, float2& point, float2& reflection, double time) const;

  bool operator<(const Bullet& other) const
  {
    return ID < other.ID;
  }
};

#pragma pack(pop)

extern std::ostream& operator<<(std::ostream& stream, const Bullet& bullet);
