#include "Common.h"

#include "PollEvents.h"

#include "Types.h"
#include "World.h"
#include "Logger.h"
#include "Config.h"
#include "SpritePawn.h"
#include "InputManager.h"
#include "WindowManager.h"
#include "ConsoleManager.h"

#include <SDL.h>


bool PollEvents(World& world)
{
  SDL_Event event;
  float2 render_offset = world.GetRenderOffset() * Config::RenderScale;

  while (SDL_PollEvent(&event))
  {
    double event_time_point = world.CurrentTime;

    switch (event.type)
    {
    case SDL_QUIT:
    {
      return false;
    }
    break;
    case SDL_KEYUP:
    {
      world.GetManager<InputManager>()->KeyReleased(
        InputManager::InputButton(InputManager::KEYBOARD, event.key.keysym.sym),
        event_time_point);
    }
    break;
    case SDL_KEYDOWN:
    {
      if (world.GetManager<ConsoleManager>()->Visible &&
        (!(event.key.keysym.scancode >= SDL_SCANCODE_F1 &&
          event.key.keysym.scancode <= SDL_SCANCODE_F12)))
      {
        world.GetManager<ConsoleManager>()->KeyPressed(event.key.keysym);
      }
      else
      {
        world.GetManager<InputManager>()->KeyPressed(
          InputManager::InputButton(InputManager::KEYBOARD, event.key.keysym.sym),
          event_time_point);
      }
    }
    break;
    case SDL_MOUSEBUTTONDOWN:
    {
      world.GetManager<InputManager>()->MouseButtonPressed(
        InputManager::InputButton(InputManager::MOUSE, event.button.button),
        (float2(event.button.x, event.button.y) - render_offset) / Config::RenderScale,
        event_time_point);
    }
    break;
    case SDL_MOUSEBUTTONUP:
    {
      world.GetManager<InputManager>()->MouseButtonReleased(
        InputManager::InputButton(InputManager::MOUSE, event.button.button),
        (float2(event.button.x, event.button.y) - render_offset) / Config::RenderScale,
        event_time_point);
    }
    break;
    case SDL_MOUSEMOTION:
    {
      world.GetManager<InputManager>()->MouseMoved(
        (float2(event.button.x, event.button.y) - render_offset) / Config::RenderScale,
        event_time_point);
      world.GetPawn<SpritePawn>()->Fire.Target =
        (float2(event.button.x, event.button.y) - render_offset) / Config::RenderScale;
    }
    break;
    case SDL_MOUSEWHEEL:
    {
      world.GetManager<InputManager>()->MouseWheel(
        event.wheel.y,
        (float2(event.button.x, event.button.y) - render_offset) / Config::RenderScale,
        event_time_point);
    }
    break;
    case SDL_WINDOWEVENT:
    {
      switch (event.window.event)
      {
      case SDL_WINDOWEVENT_RESIZED:
      case SDL_WINDOWEVENT_SIZE_CHANGED:
      case SDL_WINDOWEVENT_MAXIMIZED:
      case SDL_WINDOWEVENT_MINIMIZED:
      case SDL_WINDOWEVENT_RESTORED:
        world.GetManager<WindowManager>()->UpdateRenderResolution();
        break;
      }
    }
    break;
    }
  }

  return true;
}
