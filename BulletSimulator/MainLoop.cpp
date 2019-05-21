#include "Common.h"

#include "MainLoop.h"

#include "Math.h"
#include "World.h"
#include "Logger.h"
#include "Average.h"
#include "Rendering.h"
#include "SpritePawn.h"
#include "PollEvents.h"
#include "InputManager.h"
#include "WindowManager.h"
#include "ConsoleManager.h"
#include "NetworkManager.h"
#include "OverlayManager.h"


extern float target_render_scale;
extern const float averaging_duration;
extern Average frame_time_average;
extern Average sim_time_average;
extern Average render_time_average;


bool MainLoop(World& world, SDL_Renderer* renderer, double delta_time)
{
  Config::RenderScale = Lerp(Config::RenderScale, target_render_scale, 
    Clamp(delta_time * 5.0f * Max(1.0f, Config::RenderScale)));

  if (world.GetManager<ConsoleManager>()->ExitRequested)
    return false;

  if (!PollEvents(world))
  {
    LOG_ERROR << "main loop: poll failed";
    return false;
  }

  delta_time *= Config::TimeSpeedScale;

  world.GetManager<InputManager>()->Update(delta_time);

  for (auto& pawn : world.Pawns)
  {
    pawn->ApplyMovement(delta_time);
  }

  {
    std::scoped_lock<std::mutex> lock(world.GetManager<NetworkManager>()->EventQueueMutex);

    if (world.GetManager<NetworkManager>()->EventQueue.size())
    {
      auto& event_queue = world.GetManager<NetworkManager>()->EventQueue;

      for (auto& event : event_queue)
      {
        world.History.ScheduleEvent(event);
      }

      event_queue.clear();
    }
  }

  auto sim_start_time = std::chrono::system_clock::now();

  if (Config::ReverseTime)
  {
    world.Rewind(world.CurrentTime - delta_time);
  }
  else
  {
    world.Simulate(world.CurrentTime + delta_time);
  }
  auto sim_end_time = std::chrono::system_clock::now();
  auto sim_duration_ns = double(std::chrono::duration_cast<std::chrono::nanoseconds>(sim_end_time - sim_start_time).count());
  sim_time_average.Add(1, sim_duration_ns / 1e9);

  auto render_start_time = std::chrono::system_clock::now();

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  float2 render_offset = world.GetRenderOffset();

  SDL_RenderSetScale(renderer, Config::RenderScale, Config::RenderScale);

  for (auto& pawn : world.Pawns)
  {
    pawn->Render(renderer, world);
  }

  world.RenderCenter = Lerp(
    world.RenderCenter,
    world.GetPawn<SpritePawn>()->Location * -1.0f,
    Clamp(delta_time * Config::CameraInterpolationSpeed));

  RenderBullets(renderer, render_offset);

  RenderWalls(renderer, render_offset);

  RenderDebugOverlay(renderer);

  SDL_RenderSetScale(renderer, 1, 1);

  RenderDebugHistory(renderer);

  world.GetManager<ConsoleManager>()->Render(renderer);

  world.GetManager<OverlayManager>()->Render(renderer);

  SDL_RenderPresent(renderer);

  auto render_end_time = std::chrono::system_clock::now();
  auto render_duration_ns = double(std::chrono::duration_cast<std::chrono::nanoseconds>(render_end_time - render_start_time).count());
  render_time_average.Add(1, render_duration_ns / 1e9);

  return true;
}