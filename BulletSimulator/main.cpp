#include "Common.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include <cmath>
#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>
#include <functional>

#include "Rect.h"
#include "Draw.h"
#include "Types.h"
#include "World.h"
#include "Math.h"
#include "Config.h"
#include "Logger.h"
#include "Math2D.h"
#include "Average.h"
#include "EventsHistory.h"
#include "MainLoop.h"
#include "SpritePawn.h"
#include "StringUtils.h"
#include "InputManager.h"
#include "WindowManager.h"
#include "BulletManager.h"
#include "NetworkManager.h"
#include "OverlayManager.h"
#include "ConsoleManager.h"
#include "ProceduralTexture.h"
#include "ProceduralTextureCache.h"


Wall wall_template;
bool editing_wall = false;

float target_render_scale = 1.0;

const float averaging_duration = 0.2f;
Average frame_time_average(averaging_duration);
Average sim_time_average(averaging_duration);
Average render_time_average(averaging_duration);

static const double NS_IN_SECONDS = float(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)).count());

#ifdef _MSVC_LANG
#include <Windows.h>
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR argv, int nShowCmd)
#else
int main(int argc, const char** argv)
#endif
{
  Config::BinaryPath = argv[0];

  if (SDL_Init(SDL_INIT_VIDEO) == 0)
  {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    bool fullscreen = false;

    World world;

    { // Init InputManager
      InputManager* input_manager = new InputManager();

      auto& InputMap = input_manager->InputMap;

      // Fire
      InputMap[InputManager::InputButtonAction(InputManager::InputButton(InputManager::MOUSE, 1), InputManager::PRESSED)]
        = InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_SPACE)] 
        = InputManager::FIRE_ON;
      InputMap[InputManager::InputButtonAction(InputManager::InputButton(InputManager::MOUSE, 1), InputManager::RELEASED)]
        = InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_SPACE)]
        = InputManager::FIRE_OFF;

      // Place wall
      {
        auto button = InputManager::InputButton(InputManager::MOUSE, 3);
        InputMap[InputManager::InputButtonAction(button, InputManager::PRESSED)] = InputManager::WALL_START;
        InputMap[InputManager::InputButtonAction(button, InputManager::RELEASED)] = InputManager::WALL_END;
        InputMap[InputManager::InputButtonAction(InputManager::InputButton(InputManager::MOUSE, -1), InputManager::HOLD)] = InputManager::WALL_MOVE;
      }

      // Toggle laser sight
      InputMap[InputManager::InputButtonAction(
        InputManager::InputButton(InputManager::MOUSE, 2),
        InputManager::PRESSED)] = InputManager::LASER_TOGGLE;

      // Toggle fullscreen
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_F11)] =
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_RETURN)] = InputManager::FULLSCREEN;

      // Debug overlay
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_QUOTE)] = 
        InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_F10)] = InputManager::DEBUG_OVERLAY;

      // EventsHistory debug overlay
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_SEMICOLON)] = 
        InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_F9)] = InputManager::DEBUG_HISTORY;

      // Quit
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_ESCAPE)] = InputManager::QUIT;

      // Left
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_LEFT)] 
        = InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_a)] 
        = InputManager::InputAction(InputManager::AXIS, InputManager::HORIZONTAL, true);

      // Right
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_RIGHT)]
        = InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_d)]
        = InputManager::InputAction(InputManager::AXIS, InputManager::HORIZONTAL);

      // Up
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_UP)] 
        = InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_w)] 
        = InputManager::InputAction(InputManager::AXIS, InputManager::VERTICAL, true);

      // Down
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_DOWN)] 
        = InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_s)] 
        = InputManager::InputAction(InputManager::AXIS, InputManager::VERTICAL);

      // Console
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_BACKQUOTE)]
        = InputManager::CONSOLE;

      // Time reversal
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_TAB)]
        = InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_BACKSPACE)]
        = InputManager::TIME_REVERSE_ON;
      InputMap[InputManager::InputButtonAction(InputManager::InputButton(InputManager::KEYBOARD, SDLK_TAB), InputManager::RELEASED)]
        = InputMap[InputManager::InputButtonAction(InputManager::InputButton(InputManager::KEYBOARD, SDLK_BACKSPACE), InputManager::RELEASED)]
        = InputManager::TIME_REVERSE_OFF;

      // Time speed
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_0)] = InputManager::TIME_SPEED_0X;
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_1)] = InputManager::TIME_SPEED_1X;
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_2)] = InputManager::TIME_SPEED_3X;
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_3)] = InputManager::TIME_SPEED_5X;
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_4)] = InputManager::TIME_SPEED_10X;
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_5)] = InputManager::TIME_SPEED_25X;
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_6)] = InputManager::TIME_SPEED_50X;
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_7)] = InputManager::TIME_SPEED_100X;
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_8)] = InputManager::TIME_SPEED_500X;
      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_9)] = InputManager::TIME_SPEED_1000X;        
      
      static const auto set_time_scale = [](double value)
      {
        if (SDL_GetModState() & KMOD_SHIFT) 
          Config::TimeSpeedScale = 1.0 / value;
        else
          Config::TimeSpeedScale = value;
      };

      input_manager->ActionDelegates[InputManager::TIME_SPEED_0X] = [](double time) { Config::TimeSpeedScale = 0.0; };
      input_manager->ActionDelegates[InputManager::TIME_SPEED_1X] = [](double time) { set_time_scale(1.0); };
      input_manager->ActionDelegates[InputManager::TIME_SPEED_3X] = [](double time) { set_time_scale(3.0); };
      input_manager->ActionDelegates[InputManager::TIME_SPEED_5X] = [](double time) { set_time_scale(5.0); };
      input_manager->ActionDelegates[InputManager::TIME_SPEED_10X] = [](double time) { set_time_scale(10.0); };
      input_manager->ActionDelegates[InputManager::TIME_SPEED_25X] = [](double time) { set_time_scale(25.0); };
      input_manager->ActionDelegates[InputManager::TIME_SPEED_50X] = [](double time) { set_time_scale(50.0); };
      input_manager->ActionDelegates[InputManager::TIME_SPEED_100X] = [](double time) { set_time_scale(100.0); };
      input_manager->ActionDelegates[InputManager::TIME_SPEED_500X] = [](double time) { set_time_scale(500.0); };
      input_manager->ActionDelegates[InputManager::TIME_SPEED_1000X] = [](double time) { set_time_scale(1000.0); };

      InputMap[InputManager::InputButton(InputManager::KEYBOARD, SDLK_F5)] = InputManager::RESET_WORLD;
      input_manager->ActionDelegates[InputManager::RESET_WORLD] = [](double time) 
      { 
        World::Reset();
      };

      input_manager->ActionDelegates[InputManager::DEBUG_OVERLAY] = [](double time)
      {
        Config::ShowDebugMarkers = !Config::ShowDebugMarkers;
        LOG << "ShowDebugMarkers = " << (Config::ShowDebugMarkers ? "true" : "false");
      };

      input_manager->ActionDelegates[InputManager::DEBUG_HISTORY] = [](double time)
      {
        Config::ShowDebugHistory = !Config::ShowDebugHistory;
        LOG << "ShowDebugHistory = " << (Config::ShowDebugHistory ? "true" : "false");
      };

      input_manager->ActionDelegates[InputManager::TIME_REVERSE_ON] = [](double time)
      {
        Config::ReverseTime = true;
        LOG << "reverse time: " << (Config::ReverseTime ? "true" : "false");
      };

      input_manager->ActionDelegates[InputManager::TIME_REVERSE_OFF] = [](double time)
      {
        Config::ReverseTime = false;
        LOG << "reverse time: " << (Config::ReverseTime ? "true" : "false");
      };

      input_manager->ActionDelegates[InputManager::CONSOLE] = [](double time)
      {
        World::Get().GetManager<ConsoleManager>()->ToggleVisibility();
      };
      
      input_manager->MouseActionDelegates[InputManager::FIRE_ON] = [](int button, const float2& location, double time)
      {
        SpritePawn* pawn = World::Get().GetPawn<SpritePawn>(0);
        if (pawn) pawn->StartFiring(time, location);
      };

      input_manager->MouseActionDelegates[InputManager::FIRE_OFF] = [](int button, const float2& location, double time)
      {
        SpritePawn* pawn = World::Get().GetPawn<SpritePawn>(0);
        if (pawn) pawn->StopFiring(time);
      };      

      input_manager->MouseActionDelegates[InputManager::LASER_TOGGLE] = [](int button, const float2& location, double time)
      {
        Config::ShowViewLine = !Config::ShowViewLine;
        LOG << "ShowViewLine = " << Config::ShowViewLine;
      };

      input_manager->MouseActionDelegates[InputManager::WALL_START] = [](int button, const float2& location, double time)
      {
        wall_template.Ends = LineSegment(location, location);
        editing_wall = true;
      };

      input_manager->MouseActionDelegates[InputManager::WALL_MOVE] = [](int button, const float2& location, double time)
      {
        if (editing_wall) 
        {
          wall_template.Ends.B = location;
        }
      };

      input_manager->MouseActionDelegates[InputManager::WALL_END] = [](int button, const float2& location, double time)
      {
        if (editing_wall)
        {
          if (wall_template.Ends.A.DistanceTo(location) > 1)
          {
            wall_template.ID = World::Get().GetManager<BulletManager>()->GetNextWallID();
            wall_template.Ends.B = location;
            wall_template.Time.SetTime(time);
            World::Get().History.ScheduleEvent<EventsHistory::Add<Wall>>(time, wall_template);
            World::Get().GetManager<NetworkManager>()->AddWall(wall_template);
          }
        }
        editing_wall = false;
      };

      input_manager->MouseWheelDelegates += [&](float amount, const float2& location, double time)
      {
        if (amount > 0)
        {
          if ((SDL_GetModState() & KMOD_ALT) || (SDL_GetModState() & KMOD_SHIFT))
            Config::TimeSpeedScale *= 1.1f;
          else
            target_render_scale *= 1.1f;
        }
        else
        {
          if ((SDL_GetModState() & KMOD_ALT) || (SDL_GetModState() & KMOD_SHIFT))
            Config::TimeSpeedScale /= 1.1f;
          else
            target_render_scale /= 1.1f;
        }
      };

      world.Managers += input_manager;
    }

    world.Managers += new ConsoleManager();

    world.Managers += new NetworkManager();

    world.Managers += new BulletManager();

    world.Managers += new OverlayManager();

    {
      const float text_size = 14;
      const Color text_color = { 0, 0xFF, 0xFF, 0xFF };

      float2 offset = { 2 , 0 };

      size_t label_count = 0;

      auto add_label = [&](std::function<void(std::stringstream& stream)> generator)
      {
        world.GetManager<OverlayManager>()->AddLabel(
          float2(0, label_count++ * text_size) + offset, 
          generator, text_size, text_color);
      };

      auto add_label_color = [&](std::function<void(std::stringstream& stream, Color& color)> generator)
      {
        world.GetManager<OverlayManager>()->AddLabel(float2(0, label_count++ * text_size) + offset, generator, text_size);
      };

      add_label([](std::stringstream& stream)
      {
        size_t pixel_count = World::Get().GetManager<ConsoleManager>()->GetTextCacheSize();
        double used_fraction = double(pixel_count) / double(Config::TextCacheMaxPixels);
        double hit_rate = World::Get().GetManager<ConsoleManager>()->GetTextCacheHitRate();

        stream
          << "console font cache: "
          << String::FormatBytes(pixel_count * 4)
          << " " << String::FormatPercent(used_fraction) << "%"
          << ", hits: " << String::FormatPercent(hit_rate) << "%";
      });

      add_label([](std::stringstream& stream)
      {
        size_t pixel_count = World::Get().GetManager<OverlayManager>()->GetTextCacheSize();
        double used_fraction = double(pixel_count) / double(Config::TextCacheMaxPixels);
        double hit_rate = World::Get().GetManager<OverlayManager>()->GetTextCacheHitRate();

        stream
          << "overlay font cache: "
          << String::FormatBytes(pixel_count * 4)
          << " " << String::FormatPercent(used_fraction) << "%"
          << ", hits: " << String::FormatPercent(hit_rate) << "%";
      });

      add_label([](std::stringstream& stream)
      {
        size_t pixel_count = Draw::GetTextCacheSize();
        double used_fraction = double(Draw::GetTextCacheSize()) / double(Config::TextCacheMaxPixels);
        double hit_rate = Draw::GetTextCacheHitRate();

        stream
          << "   draw font cache: "
          << String::FormatBytes(pixel_count * 4)
          << " " << String::FormatPercent(used_fraction) << "%"
          << ", hits: " << String::FormatPercent(hit_rate) << "%";
      });

      add_label([](std::stringstream& stream)
      {        
        stream
          << "     texture cache: "
          << String::FormatBytes(ProceduralTextureCache::Get().TotalSize * 4);
      });

      add_label([](std::stringstream& stream)
      {
        float used_percent = float(World::Get().History.GetTotalSize()) / float(Config::HistoryMaxBytes) * 100.0f;

        std::string diff = "0.00";
        if (World::Get().History.EventsLog.size())
        {
          diff = String::Format(World::Get().CurrentTime - World::Get().History.EventsLog.Last()->Time, 2);
        }
        stream
          << "   history: " << String::FormatBytes(World::Get().History.GetTotalSize())
          << " " << String::Format(used_percent, 0) << "%, " << diff << "s";
      });

      add_label([](std::stringstream& stream)
      {
        stream << "    events: "
          << World::Get().History.EventsQueue.size() << " future, "
          << World::Get().History.EventsLog.size() << " past";
      });

      add_label([](std::stringstream& stream)
      {
        stream
          << "   bullets: " << World::Get().Bullets.size()
          << ", walls: " << World::Get().Walls.size();
      });

      add_label([](std::stringstream& stream)
      {
        float used_percent = float(World::Get().History.GetTotalSize()) / float(Config::HistoryMaxBytes) * 100.0f;
        char str[64];
        snprintf(str, sizeof(str), "%.2lf", World::Get().CurrentTime);
        stream << "world time: " << std::string(str);
      });

      add_label_color([](std::stringstream& stream, Color& color)
      {
        stream << "time speed: " << String::FormatPercentDynamic(Config::TimeSpeedScale) << "x";
        color = Color::CYAN.Mix(Color::YELLOW, Config::TimeSpeedScale / 100.0)
                           .Mix(Color::WHITE.WithGB(127), Config::TimeSpeedScale / 1000.0);
      });

      add_label([](std::stringstream& stream)
      {
        auto& res = World::Get().GetManager<WindowManager>()->RenderResolution;
        stream << "resolution: " << res.x << "x" << res.y;
      });

      /*
      add_label([](std::stringstream& stream)
      {
        Average average;

        BulletManager* manager = World::Get().GetManager<BulletManager>();

        float rate = average.Add(manager->Stats.CollisionCount, manager->Stats.LastUpdateDeltaTime);
        
        rate *= Config::TimeSpeedScale;

        char str[64];
        snprintf(str, sizeof(str), "%.2lf", rate);
        stream  << "collision rate: " << str << "/s";        
      });
      */

      static const auto fps_color = [](float value, float multiplier = 1.0f) -> Color
      {
        float ratio = Clamp(value / (multiplier / Config::TargetFPS * 1000.0f));
        ratio = pow(ratio, 2.0f);
        if (ratio < 0.5f)
          return Color::CYAN.Mix(Color::YELLOW.WithBlue(50), ratio * 2.0f);
        else
          return Color::YELLOW.WithBlue(50).Mix(Color::RED.WithGB(50), ratio * 2.0f - 1.0f);
      };

      add_label_color([](std::stringstream& stream, Color& color)
      {
        float value = 1000.0 / frame_time_average.Rate;
        stream << "frame time: " << String::Format(value, 2) << " ms";
        color = fps_color(value);
      });
      add_label_color([](std::stringstream& stream, Color& color)
      {
        float value = 1000.0 / sim_time_average.Rate;
        stream << "  sim time: " << String::Format(value, 2) << " ms";
        color = fps_color(value);
      });
      add_label_color([](std::stringstream& stream, Color& color)
      {
        float value = 1000.0 / render_time_average.Rate;
        stream << " draw time: " << String::Format(value, 2) << " ms";
        color = fps_color(value);
      });
    }

    SpritePawn* pawn = new SpritePawn(0, renderer);

    world.Pawns += pawn;

    world.GetManager<InputManager>()->ActionDelegates[InputManager::FULLSCREEN] = [&world](double time)
    {
      world.GetManager<WindowManager>()->ToggleFullscreen();
    };

    world.GetManager<InputManager>()->AxisActionDelegates[InputManager::HORIZONTAL] = [&pawn](float value, double time)
    {
      pawn->Move(float2(1.0f * value, 0));
    };

    world.GetManager<InputManager>()->AxisActionDelegates[InputManager::VERTICAL] = [&pawn](float value, double time)
    {
      pawn->Move(float2(0, 1.0f * value));
    };
    
    std::chrono::system_clock::time_point last_frame_time = std::chrono::system_clock::now();
    
    if (SDL_CreateWindowAndRenderer(1024, 768, 0, &window, &renderer) == 0) 
    {
      SDL_SetWindowTitle(window, "Bullet Simulator");

      SDL_SetWindowResizable(window, SDL_TRUE);

      //SDL_ShowCursor(SDL_DISABLE);

      bool done = false;
      
      world.GetManager<InputManager>()->ActionDelegates[InputManager::InputAction(InputManager::QUIT)] = 
        [&done](double time)
      {
        done = true;
      };

      world.Managers += new WindowManager(window, renderer);

      while (!done)
      {
        auto sleep_duration = (last_frame_time + std::chrono::seconds(1) / Config::TargetFPS) - std::chrono::system_clock::now();
        if (sleep_duration.count() > 0)
        {
          std::this_thread::sleep_for(sleep_duration);
        }

        auto frame_start_time = std::chrono::system_clock::now();

        double delta_time = double(std::chrono::duration_cast<std::chrono::nanoseconds>(frame_start_time - last_frame_time).count()) / NS_IN_SECONDS;

        {
          std::scoped_lock<std::mutex> lock(world.MainLoopMutex);
          if (!MainLoop(world, renderer, delta_time))
          {
            done = true;
          }
        }

        auto frame_end_time = std::chrono::system_clock::now();

        auto frame_duration_ns = double(std::chrono::duration_cast<std::chrono::nanoseconds>(frame_end_time - frame_start_time).count());
        
        frame_time_average.Add(1, frame_duration_ns / NS_IN_SECONDS);

        last_frame_time = frame_start_time;
      }
    }

    if (renderer) 
    {
      SDL_DestroyRenderer(renderer);
    }
    if (window) 
    {
      SDL_DestroyWindow(window);
    }
  }

  SDL_Quit();

  return 0;
}
