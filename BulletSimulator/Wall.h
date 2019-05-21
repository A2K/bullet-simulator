#pragma once

#include "Vector2.h"
#include "ProtocolEnums.h"
#include "Timestamp.h"
#include "LineSegment.h"


#pragma pack(push, 4)

struct Wall
{
  typedef uint32_t id_t;

  id_t ID;
  LineSegment Ends;
  Timestamp<> Time;
  
  static const uint8_t EntityType = Protocol::EntityType::WALL;

  Wall() :
    Time(0.0f)
  {}

  Wall(const id_t id, const LineSegment& segment, const float time) :
    ID(id), Ends(segment), Time(time)
  {}

  bool operator<(const Wall& other) const
  {
    return ID < other.ID;
  }
};

#pragma pack(pop)
