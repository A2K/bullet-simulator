#pragma once

#include "Types.h"


class Pawn
{
public:

  unsigned int ID = 0;

  float2 Location;

  explicit Pawn(unsigned int id = 0, const float2& location = float2(0, 0)) :
    ID(id), Location(location)
  {}

  virtual ~Pawn()
  {}

  virtual void Move(const float2& direction) {};

  virtual void ApplyMovement(double delta_time) {};

  virtual void Render(struct SDL_Renderer* renderer, class World& world) = 0;
};

