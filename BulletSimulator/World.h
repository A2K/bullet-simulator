#pragma once

#include "List.h"
#include "Wall.h"
#include "Types.h"
#include "Bullet.h"
#include "Config.h"
#include "EventsHistory.h"
#include "IndexMap.h"

#include <mutex>
#include <chrono>
#include <functional>


class World
{
private: 

  static World* instance;

public:

  static inline constexpr World& Get()
  {
    return *(instance ? instance : (instance = new World()));
  }

  static inline constexpr double& Time()
  {
    return Get().CurrentTime;
  }

  static void Reset();
  
  const std::chrono::system_clock::time_point StartTimePoint;

  const uint32_t StartTicks;

  double CurrentTime = 0.0;

  explicit World(std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now());

  ~World();  

  IndexMap<Bullet::id_t, Bullet> Bullets;
  IndexMap<Wall::id_t, Wall> Walls;
  
  List<class Pawn*> Pawns;
  List<class Manager*> Managers;

  template<class PawnClass>
  inline PawnClass* GetPawn(int index = 0)
  {
    for (auto pawn : Pawns)
    {
      PawnClass* instance = dynamic_cast<PawnClass*>(pawn);
      if (instance && (--index <= 0)) return instance;
    }
    return nullptr;
  }

  template<class ManagerClass>
  inline ManagerClass* GetManager()
  {
    for (auto manager : Managers)
    {
      ManagerClass* instance = dynamic_cast<ManagerClass*>(manager);
      if (instance) return instance;
    }
    return nullptr;
  }

  float2 RenderCenter = float2(0, 0);

  float2 GetRenderOffset();
  
  EventsHistory History;

  std::mutex MainLoopMutex;
  
public:

  void Simulate(double time);

  void Rewind(double time);
  
};
