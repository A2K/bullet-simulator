#include "Common.h"

#include "ConsoleManager.h"

#include "Draw.h"
#include "Math.h"
#include "Rect.h"
#include "Color.h"
#include "Array.h"
#include "Logger.h"
#include "Config.h"
#include "StringUtils.h"
#include "BulletManager.h"
#include "WindowManager.h"
#include "NetworkClient.h"
#include "NetworkManager.h"

#include <cmath>
#include <ctime> 
#include <string>   
#include <cctype>
#include <vector>
#include <istream>
#include <sstream>
#include <iterator>
#include <algorithm> 
#include <functional>


std::string GetTimeString(std::chrono::system_clock::time_point time = std::chrono::system_clock::now())
{
  time_t tt = std::chrono::system_clock::to_time_t(time);
  char buf[256];
  ctime_s(buf, 256, &tt);
  return std::string(buf);
}

ConsoleManager::ConsoleManager()
{
  AddMessage("Press [~] to open console");
}

struct Ops
{
  std::function<std::string()> Get;
  std::function<void(const std::string&)> Set;
};

Map<std::string, Ops>& GetConfigOps()
{
  static Map<std::string, Ops> config_ops;  

  if (config_ops.size())
  {
    return config_ops;
  }

  static const auto config_var_size_t = [](const std::string& name, size_t& value)
  {
    config_ops[name] = { [&value]()
    {
      return std::to_string(value);
    }, [&value](const std::string new_value) {
      value = std::stoi(new_value);
    } };
  };

  static const auto config_var_float = [](const std::string& name, float& value)
  {
    config_ops[name] = { [&value]()
    {
      return std::to_string(value);
    }, [&value](const std::string new_value) {
      try
      {
        value = std::stof(new_value);
      }
      catch (std::exception&)
      {
        LOG_ERROR << "invalid value: " << new_value << " expected: float";
      }
    } };
  };

  static const auto config_var_double = [](const std::string& name, double& value)
  {
    config_ops[name] = { [&value]()
    {
      return std::to_string(value);
    }, [&value](const std::string new_value) {
      try
      {
        value = std::stof(new_value);
      } 
      catch(std::exception&)
      {
        LOG_ERROR << "invalid value: " << new_value << " expected: float";
      }
    } };
  };

  static const auto config_var_bool = [](const std::string& name, bool& value)
  {
    config_ops[name] = { [&value]()
    {
      return value ? "true" : "false";
    }, [&value](std::string new_value) {
      String::Trim(new_value);
      std::transform(new_value.begin(), new_value.end(), new_value.begin(), ::tolower);
      value = (new_value == "true"
        || new_value == "yes"
        || new_value == "enable"
        || new_value == "enabled"
        || new_value == "on"
        || (String::IsNumber(new_value) && std::stoi(new_value) > 0));
    } };
  };

  config_var_bool("DestroyWallsOnCollision", Config::DestroyWallsOnCollision);
  config_var_bool("ReverseTime", Config::ReverseTime);
  config_var_bool("ShowDebugMarkers", Config::ShowDebugMarkers);    
  config_var_bool("ShowViewLine", Config::ShowViewLine);
  config_var_bool("ShowViewCollisions", Config::ShowViewCollisions);
  config_var_bool("EnableParallelTextureGeneration", Config::EnableParallelTextureGeneration);  
  config_var_bool("ShowMouseLocation", Config::ShowMouseLocation);
  config_var_bool("LogCollisions", Config::LogCollisions);
  config_var_bool("LogNetwork", Config::LogNetwork);
  
  config_var_double("Time", World::Get().CurrentTime);
  config_var_double("TimeSpeedScale", Config::TimeSpeedScale);
  config_var_double("BulletSpeed", Config::BulletSpeed);
  config_var_double("HistoryMaxAge", Config::HistoryMaxAge);
  config_var_double("FireRate", Config::FireRate);
  config_var_double("BulletRadius", Config::BulletRadius);
  config_var_double("BaseLineWidth", Config::BaseLineWidth);
  config_var_double("CameraInterpolationSpeed", Config::CameraInterpolationSpeed);  
  config_var_double("Acceleration", Config::Acceleration);
  config_var_double("MovementSpeed", Config::MovementSpeed);
  
  config_var_double("DebugValue1", Config::DebugValue1);    
  config_var_double("DebugValue2", Config::DebugValue2);
  config_var_double("DebugValue3", Config::DebugValue3);

  config_var_float("RenderScale", Config::RenderScale);  
  config_var_float("TargetFPS", Config::TargetFPS);

  config_var_size_t("HistoryMaxBytes", Config::HistoryMaxBytes);
  config_var_size_t("TextCacheMaxPixels", Config::TextCacheMaxPixels);    
  config_var_size_t("MaxPreciseBullets", Config::MaxPreciseBullets);
  
  return config_ops;
}

Map<std::string, std::function<void(const std::vector<std::string>& args)>>& ConsoleManager::GetCommands() 
{
  if (Commands.size())
  {
    return Commands;
  }

  Commands["help"] = [this](const std::vector<std::string>& args)
  {
    AddMessage("available commands:");
    AddMessage("\t      help              print this information");
    AddMessage("\t     clear              clear console");
    AddMessage("\t     reset              reset the world");
    AddMessage("\t      time              print current time");
    AddMessage("\t       set KEY VALUE    set config variable value");
    AddMessage("\t       get KEY          get config variable value");
    AddMessage("\t      list              print all variables and their values");
    AddMessage("\t      time              print current time");
    AddMessage("\tfullscreen [on|off]     toggle fullscreen");
    AddMessage("\t    listen PORT         start a network server");
    AddMessage("\t      host PORT         ");
    AddMessage("\t      bind PORT         ");
    AddMessage("\t   connect [ADDR] PORT  connect to a network server");
    AddMessage("\t      join [ADDR] PORT  ");
    AddMessage("\t      sync              request world update from server");
    AddMessage("\t      quit              exit");
    AddMessage("\t      exit              ");
  };

  Commands["quit"] = 
    Commands["exit"] = [this](const std::vector<std::string>& args)
  {
    AddMessage("exiting");
    ExitRequested = true;
  };

  Commands["clear"] = [this](const std::vector<std::string>& args)
  {
    Messages.clear();
  };

  Commands["time"] = [this](const std::vector<std::string>& args)
  {
    MessageStream(*this) << "current time: " << GetTimeString();
    MessageStream(*this) << "seconds since start: " << World::Get().CurrentTime;
  };

  Commands["fullscreen"] = [this](const std::vector<std::string>& args)
  {
    if (args.size() == 1)
    {
      std::string arg = args[0];
      std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
      if (arg == "on")
      {
        World::Get().GetManager<WindowManager>()->Fullscreen();
      }
      else if (arg == "off")
      {
        World::Get().GetManager<WindowManager>()->Windowed();
      }
    }
    else
    {
      World::Get().GetManager<WindowManager>()->ToggleFullscreen();
    }
    AddMessage((World::Get().GetManager<WindowManager>()->IsFullscreen ? "fullscreen" : "windowed") + std::string(" mode"));
  };

  Commands["list"] = [this](const std::vector<std::string>& args)
  {
    for (auto pair : GetConfigOps())
    {
      MessageStream(*this) << pair.first << " = " << pair.second.Get();
    }
  };

  Commands["get"] = [this](const std::vector<std::string>& args)
  {
    if (args.size() != 1)
    {
      LOG_ERROR << "invalid number of arguments: " << args.size() << " expected: 1";
      return;
    }
    auto iter = GetConfigOps().find(args[0]);
    if (iter == GetConfigOps().end())
    {
      LOG_ERROR << "unknown variable: " << args[0];
      return;
    }

    MessageStream(*this) << args[0] << ": " << iter->second.Get();
  };

  Commands["set"] = [this](const std::vector<std::string>& args)
  {
    if (args.size() < 2)
    {
      LOG_ERROR << "invalid number of arguments: " << args.size() << " expected: 2";
      return;
    }
    auto iter = GetConfigOps().find(args[0]);
    if (iter == GetConfigOps().end())
    {
      LOG_ERROR << "unknown variable: " << args[0];
      return;
    }

    iter->second.Set(args[1]);

    MessageStream(*this) << args[0] << " = " << args[1];
  };

  Commands["reset"] = [this](const std::vector<std::string>& args)
  {
    World::Reset();
    LOG << "world reset";
  };

  Commands["connect"] = 
  Commands["join"] = [this](const std::vector<std::string>& args)
  {
    if (args.size() == 1)
    {
      if (!String::IsNumber(args[0]))
      {
        LOG_ERROR << "invalid argument: " << args[0] << " expected: port number";
        return;
      }
      World::Get().GetManager<NetworkManager>()->Connect("127.0.0.1", std::stoi(args[0]));
    }
    else if(args.size() == 2)
    {
      if (!String::IsNumber(args[1]))
      {
        LOG_ERROR << "invalid argument: " << args[1] << " expected: port number";
        return;
      }
      World::Get().GetManager<NetworkManager>()->Connect(args[0], std::stoi(args[1]));
    }
    else
    {
      LOG_ERROR << "invalid number of arguments: " << args.size() << " expected: 1 or 2";
    }
  };

  Commands["listen"] =
  Commands["host"] =
  Commands["bind"] = [this](const std::vector<std::string>& args)
  {
    World::Get().GetManager<NetworkManager>()->Listen(std::stof(args[0]));
  };

  Commands["sync"] = [this](const std::vector<std::string>& args)
  {
    auto& client = World::Get().GetManager<NetworkManager>()->Network.Client;
    if (client) 
    {
      client->Sync();
    }
    else
    {
      LOG_ERROR << "can't sync, not connected as a client";
    }
  }; 

  Commands["test"] = [this](const std::vector<std::string>& args)
  {
    World::Reset();

    Wall wall1 = { ++Config::LastWallId, { { 0, 0 }, { -100, 100 } }, 1.0f };
    World::Get().History.ScheduleEvent<EventsHistory::Add<Wall>>(1.0, wall1);
    Wall wall2 = { ++Config::LastWallId, { { 0, 0 }, { 100, 100 } }, 1.0f };
    World::Get().History.ScheduleEvent<EventsHistory::Add<Wall>>(1.0, wall2);

    Bullet bullet(++Config::LastBulletId, float2(0, 200), float2(0, -1), 100.0f, 1.25, 1e9);
    World::Get().GetManager<BulletManager>()->UpdateBulletCollision(bullet, 1.25);
    World::Get().History.ScheduleEvent<EventsHistory::Add<Bullet>>(1.25, bullet);
    
    Config::TimeSpeedScale = 1.0;
  };

  return Commands;
}

void ConsoleManager::AddMessage(const std::string& message, const Color& color, double max_age)
{
  Messages.push_front({ std::chrono::system_clock::now(), message, color, max_age });
  while (Messages.size() > Backlog)
  {
    Messages.pop_back();
  }
}

void ConsoleManager::RenderMessages(SDL_Renderer* renderer, const float2& location)
{  
  float y = location.y;

  Rect render_quad;

  for (const Message& message : Messages)
  {
    if (y < 0) return;

    double height = Max(0.0f, (y - FontSize)) / World::Get().GetManager<WindowManager>()->RenderResolution.y;
    height = pow(height, 0.25);

    if (height < 0.001) return;

    uint8_t alpha = 0xFF * height;

    if (!Visible)
    {

      double age = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - message.time).count() / 1e9;      
      alpha = uint8_t(0xFF * (1.0 - std::min(message.max_age, age) / message.max_age) * height);

      if (alpha == 0) continue;
    }

    float2 text_size;
    std::shared_ptr<SDL_Texture> texture = text_renderer.GetTexture(renderer, message.text, FontSize, message.color, text_size);

    SDL_SetTextureAlphaMod(texture.get(), alpha);

    y -= text_size.y;

    render_quad = Rect(int2(location.x, y), text_size);

    SDL_RenderCopy(renderer, &(*texture), nullptr, &render_quad);
  }
}

float2 ConsoleManager::RenderPrompt(SDL_Renderer* renderer, float2 location)
{  
  float2 text_size;

  std::string text = Prompt + Input;
  std::shared_ptr<SDL_Texture> texture = text_renderer.GetTexture(renderer, text, FontSize, Color::WHITE, text_size);

  SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0xFF, 0xFF);

  Rect textRenderQuad(location - float2(0, text_size.y), text_size);

  Draw::Utility::UpdateTextureTintColor(renderer, texture.get());

  SDL_RenderCopy(renderer, texture.get(), NULL, &textRenderQuad);

  if (fmodf(float(SDL_GetTicks()) / 500.0f, 1.0f) < 0.5f)
  {
    float2 text_before_cursor_size = text_renderer.GetTextSize(Prompt + Input.substr(0, CursorPosition), FontSize);
    int x = int(location.x + text_before_cursor_size.x);
    int y = int(location.y - text_before_cursor_size.y);
    SDL_RenderDrawLine(renderer, x, y, x, y + int(text_size.y));
  }
  
  return text_size;
}

void ConsoleManager::Render(SDL_Renderer* renderer)
{
  float2 offset(4, 4);
  
  float y_max = World::Get().GetManager<WindowManager>()->RenderResolution.y - offset.y;

  if (Visible)
  {
    float2 prompt_size = RenderPrompt(renderer, float2(offset.x, y_max));

    y_max -= prompt_size.y;
  }

  offset.y = y_max;

  RenderMessages(renderer, offset);

  if (Visible)
  {
    if (Suggestions.size())
    {
      float2 pos = { offset.x + text_renderer.GetTextSize(Prompt + "set ", FontSize).x, y_max };

      List<std::string> list(Suggestions.begin(), Suggestions.end());
      Draw::TextBox(renderer, pos, list, 14,
        Color::Gray(25), Color(0, 125, 125), { 6, 2 },
        false, true,
        SuggestionIndex, Color(0, 125, 125),
        Color::WHITE, Text::BLENDED);
    }
  }
}

void ConsoleManager::ToggleVisibility()
{
  Visible = !Visible;
}

void ConsoleManager::KeyPressed(SDL_Keysym& Keysym)
{
  if (!(Keysym.sym == SDLK_TAB
    || (Keysym.sym == SDLK_UP) || (Keysym.sym == SDLK_DOWN)
    || (Keysym.sym == SDLK_SPACE && Keysym.mod & KMOD_CTRL)))
  {
    Suggestions.clear();
    SuggestionIndex = -1;
  }

  auto UpdateCurrentSuggestion = [this]()
  {
    auto iter = Suggestions.begin();
    std::advance(iter, SuggestionIndex);
    Array<std::string> parts = String::Split<Array>(Input);
    bool starts_with_set = parts[0] == "set";
    Input = (starts_with_set ? "set " : "get ") + (*iter) + (parts.size() > 2 ? std::string(" ") + parts[2] : " ");
    CursorPosition = Input.size();
  };

  char ch = Keysym.sym;

  auto erase_world_before_cursor = [this]
  {
    auto pos = Input.rfind(' ', CursorPosition);
    if (pos == std::string::npos)
    {
      Input = Input.substr(CursorPosition);
    }
    else
    {
      auto prefix = Input.substr(0, pos);
      Input = String::TrimRight(prefix) + Input.substr(CursorPosition);
    };
  };

  auto erase_world_after_cursor = [this]
  {
    auto pos = Input.find(' ', CursorPosition);
    if (pos == std::string::npos)
    {
      Input = Input.substr(0, CursorPosition);
    }
    else
    {
      auto postfix = Input.substr(pos);
      Input = Input.substr(0, CursorPosition) + String::TrimLeft(postfix);
    };
  };

  if (Keysym.sym == SDLK_RETURN || Keysym.sym == SDLK_RETURN2)
  {
    ExecuteCommand(Input);
    Input.clear();
    CursorPosition = 0;
  }
  else if (Keysym.sym == SDLK_BACKSPACE)
  {
    if ((Keysym.mod & KMOD_RCTRL || Keysym.mod & KMOD_LCTRL))
    {
      erase_world_before_cursor();
    }
    else if (CursorPosition > 0)
    {
      auto iter = Input.begin();
      if (iter != Input.end())
      {
        if (CursorPosition > 1)
        {
          std::advance(iter, --CursorPosition);
        }
        if (iter != Input.end())
        {
          Input.erase(iter);
        }
      }
    }
  }
  else if (Keysym.sym == SDLK_DELETE)
  {
    if ((Keysym.mod & KMOD_RCTRL || Keysym.mod & KMOD_LCTRL))
    {
      erase_world_after_cursor();
    }
    else
    {
      auto iter = Input.begin();
      if (iter != Input.end())
      {
        std::advance(iter, CursorPosition);
        if (iter != Input.end())
        {
          Input.erase(iter);
        }
      }
    }
  }
  else if (Keysym.sym == SDLK_RIGHT)
  {
    if (Keysym.mod & KMOD_CTRL)
    {
      bool erased_spaces = false;
      while (CursorPosition < Input.size() && Input[CursorPosition] == ' ')
      {
        CursorPosition++;
        erased_spaces = true;
      }

      if (!erased_spaces)
      {
        while (CursorPosition < Input.size() && Input[CursorPosition] != ' ')
        {
          CursorPosition++;
        }
      }
    }
    else
    {
      CursorPosition++;
    }
  }
  else if (Keysym.sym == SDLK_LEFT)
  {
    if (Keysym.mod & KMOD_CTRL)
    {
      bool erased_spaces = false;
      while (CursorPosition > 0 && Input[CursorPosition - 1] == ' ')
      {
        CursorPosition--;
        erased_spaces = true;
      }

      if (!erased_spaces)
      {
        while (CursorPosition > 0 && Input[CursorPosition - 1] != ' ')
        {
          CursorPosition--;
        }
      }
    }
    else
    {
      if (CursorPosition > 0)
      {
        CursorPosition--;
      }
    }    
  }
  else if (Keysym.sym == SDLK_HOME)
  {
    CursorPosition = 0;
  }
  else if (Keysym.sym == SDLK_END)
  {
    CursorPosition = Input.size();
  }
  else if (Keysym.sym == SDLK_UP)
  {
    if (Suggestions.size())
    {
      SuggestionIndex = SuggestionIndex - 1;
      if (SuggestionIndex < 0) SuggestionIndex = Suggestions.size() - 1;
      UpdateCurrentSuggestion();
    }
    else
    {
      HistoryCursorPosition = HistoryCursorPosition < EventsHistory.size() ? HistoryCursorPosition + 1 : int(EventsHistory.size());
      auto iter = EventsHistory.begin();
      std::advance(iter, HistoryCursorPosition > 0 ? HistoryCursorPosition - 1 : 0);
      if (iter != EventsHistory.end())
      {
        Input = *iter;
      }
      CursorPosition = Input.size();
    }
  }
  else if (Keysym.sym == SDLK_DOWN)
  {
    if (Suggestions.size())
    {
      SuggestionIndex = (SuggestionIndex + 1) % Suggestions.size();
      UpdateCurrentSuggestion();
    }
    else
    {
      HistoryCursorPosition = HistoryCursorPosition <= EventsHistory.size() ?
        (HistoryCursorPosition > 0 ? HistoryCursorPosition - 1 : 0) : int(EventsHistory.size());

      auto iter = EventsHistory.begin();
      if (HistoryCursorPosition > 0)
      {
        std::advance(iter, HistoryCursorPosition > 0 ? HistoryCursorPosition - 1 : 0);
        Input = *iter;
      }
      else
      {
        Input = "";
      }
      CursorPosition = Input.size();
    }
  }
  else if (Keysym.sym == SDLK_TAB
           || (Keysym.sym == SDLK_SPACE && Keysym.mod & KMOD_CTRL))
  {

    Array<std::string> parts = String::Split<Array>(Input);

    if (parts.size())
    {
      bool starts_with_set = (parts[0] == std::string("set"));
      bool starts_with_get = (parts[0] == std::string("get"));
      if (starts_with_set || starts_with_get)
      {
        if (Suggestions.size())
        {
          SuggestionIndex = (SuggestionIndex + 1) % Suggestions.size();
          UpdateCurrentSuggestion();
        } 
        else if (parts.size() >= 1)
        {
          Set<std::string> matches;

          std::string input_var_name = parts.size() > 1 ? String::ToLower(String::Trimmed(parts[1])) : "";

          for (auto pair : GetConfigOps())
          {
            if (!input_var_name.size())
            {
              matches.insert(pair.first);
              continue;
            }

            std::string copy = pair.first.substr(0, input_var_name.size());
            if (String::ToLower(copy) == input_var_name)
            {
              matches.insert(pair.first);
            }
          }

          if (matches.size() == 1)
          {
            Input = (starts_with_set ? "set " : "get ") + (*matches.begin()) + (parts.size() > 2 ? std::string(" ") + parts[2] : " ");
            CursorPosition = Input.size();
          }
          else if (matches.size() > 1)
          {
            Suggestions = matches;
            SuggestionIndex = -1;

            if (String::ToLower(*(matches.begin())) == input_var_name)
            {
              SuggestionIndex = 0;
            }
            else
            {
              std::string prefix = String::CommonPrefix(matches);

              Input = (starts_with_set ? "set " : "get ") + prefix + (parts.size() > 2 ? std::string(" ") + parts[2] : "");
              CursorPosition = Input.size();
            }
          }
        }
        else
        {
          Input = (starts_with_set ? "set " : "get ") + GetConfigOps().begin()->first + " ";          
        }
      }
    }

    auto& commands = GetCommands();
    for (auto& pair : commands)
    {
      if (pair.first.substr(0, Input.size()) == Input)
      {
        Input = pair.first;
        CursorPosition = Input.size();
      }
    }
  }
  else if (Keysym.sym == SDLK_BACKQUOTE || Keysym.sym == SDLK_ESCAPE)
  {
    Visible = false;
  }
  else if (ch >= ' ' && ch <= '~')
  {
    if ((Keysym.mod & KMOD_RCTRL || Keysym.mod & KMOD_LCTRL) &&
        (Keysym.sym == SDLK_w || Keysym.sym == SDLK_BACKSPACE))
    { 
      erase_world_before_cursor();
    }
    else
    {
      if (Keysym.mod & KMOD_LSHIFT || Keysym.mod & KMOD_RSHIFT)
      {
        if (ch >= 'a' && ch <= 'z')
        {
          ch += 'A' - 'a';
        }
        else
        {
          
          static const Map<char, char> shift_numbers = {
            { '1', '!' },
            { '2', '@'},
            { '3', '#'},
            { '4', '$'},
            { '5', '%'},
            { '6', '^'},
            { '7', '&'},
            { '8', '*'},
            { '9', '('},
            { '0', ')'},
            { '-', '_'},
            { '=', '+'},
            { '\\', '|'},
            { '/', '?'},
            { '\'', '"'},
            { ';', ':'},
            { ',', '<'},
            { '.', '>'},
            { '[', '{'},
            { ']', '}'}
          };
          auto shift_iter = shift_numbers.find(ch);
          if (shift_iter != shift_numbers.end())
          {
            ch = shift_iter->second;
          }
        }
      }
      Input.insert(std::min(CursorPosition++, Input.size()), 1, ch);
    }
  }  

  if ((Keysym.sym != SDLK_UP) && (Keysym.sym != SDLK_DOWN))
  {
    HistoryCursorPosition = 0;
  }

  CursorPosition = std::min(CursorPosition, Input.size());
}

void ConsoleManager::ExecuteCommand(const std::string& text)
{
  if (!EventsHistory.size() || text != EventsHistory.front())
  {
    EventsHistory.push_front(text);
  }

  AddMessage(Prompt + text);
  
  Array<std::string> parts = String::Split<Array>(text);

  if (parts.size() == 0) return;

  std::string command = String::ToLower(parts[0]);

  parts.erase(parts.begin());

  auto iter = GetCommands().find(command);
  if (iter != GetCommands().end())
  {
    iter->second(parts);
  }
  else
  {
    LOG_ERROR << "invalid command: " << command;
  }
}
