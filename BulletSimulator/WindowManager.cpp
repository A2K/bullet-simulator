#include "Common.h"

#include "WindowManager.h"

#include "Logger.h"

#include <SDL.h>


WindowManager::WindowManager(SDL_Window* window, SDL_Renderer* renderer):
  Window(window),
  Renderer(renderer),
  IsFullscreen(false)
{
  UpdateRenderResolution();
}

void WindowManager::UpdateRenderResolution()
{
  int x, y;
  SDL_GetRendererOutputSize(Renderer, &x, &y);
  RenderResolution.x = x;
  RenderResolution.y = y;
}

void WindowManager::Windowed()
{
  SDL_SetWindowFullscreen(Window, 0);
  IsFullscreen = false;
  UpdateRenderResolution();
}

void WindowManager::Fullscreen()
{
  SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
  IsFullscreen = true;
  UpdateRenderResolution();
}

void WindowManager::ToggleFullscreen()
{
  if (IsFullscreen)
  {
    Windowed();
  }
  else
  {
    Fullscreen();
  }
}
