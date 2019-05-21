#pragma once

#include "Types.h"

#include <SDL.h>


extern void RenderBullets(SDL_Renderer* renderer, const float2& render_offset);

extern void RenderDebugHistory(SDL_Renderer* renderer);

extern void RenderDebugOverlay(SDL_Renderer* renderer);

extern void RenderWalls(SDL_Renderer* renderer, const float2& render_offset);
