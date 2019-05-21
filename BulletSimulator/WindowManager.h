#pragma once

#include "Types.h"
#include "Vector2.h"
#include "Manager.h"


class WindowManager : public Manager
{
public:

  struct SDL_Window* Window;
  struct SDL_Renderer* Renderer;

  bool IsFullscreen;

  WindowManager(SDL_Window* window, SDL_Renderer* renderer);

  void Windowed();

  void Fullscreen();

  void ToggleFullscreen();
  
  float2 RenderResolution;

  void UpdateRenderResolution();
  
};
