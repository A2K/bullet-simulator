#pragma once

#include "Pawn.h"
#include "Config.h"


class SpritePawn : public Pawn
{
private:

  const int2 TextureSize;

public:
  
  float2 MovementVector;

  SpritePawn(unsigned int ID, SDL_Renderer* renderer, const int2& TextureSize = int2(128, 128), 
             const float2& Location = float2(0, 0));
  
  void Move(const float2& direction) override;

  void ApplyMovement(double delta_time) override;

  void Render(SDL_Renderer* renderer, World& world) override;

  void StartFiring(double time, const float2& target);

  void StopFiring(double time);

  void UpdateFiring(double time, const float2& last_location);

  struct
  {
    double LastFireTime = 0.0;
    bool Active = false;
    float2 Target;
  } Fire;

private:

  float2 Velocity = 0;

  float2 UpdateVelocity(const float2& delta, double delta_time);

};
