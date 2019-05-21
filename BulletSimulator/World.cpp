#include "Common.h"

#include "Math.h"
#include "Pawn.h"
#include "World.h"
#include "Config.h"
#include "BulletManager.h"
#include "WindowManager.h"

#include <SDL.h>

#include <algorithm>
#include <execution>


World* World::instance = nullptr;

void World::Reset()
{
  if (instance)
  {
    instance->Bullets.clear();
    instance->Walls.clear();
    instance->History.Clear();
    instance->CurrentTime = 0;
    instance->RenderCenter = 0;
    for (auto& pawn : instance->Pawns)
    {
      pawn->Location = 0;
    }
  }
  Config::LastBulletId = 0;
  Config::LastWallId = 0;
}

World::World(std::chrono::system_clock::time_point StartTime):
  StartTimePoint(StartTime), StartTicks(SDL_GetTicks()), CurrentTime(0)
{
  instance = this;
}

World::~World()
{
  instance = nullptr;
}

float2 World::GetRenderOffset()
{
  return RenderCenter + GetManager<WindowManager>()->RenderResolution * 0.5f / Config::RenderScale;
}

void World::Simulate(double time)
{
  History.ProcessEventsQueue(time);
  History.Cleanup();
  CurrentTime = time;
}

void World::Rewind(double time)
{
  History.Rewind(time);
  
  CurrentTime = time;
}
