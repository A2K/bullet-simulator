#pragma once

#include <SDL.h>

#include "Color.h"
#include "World.h"
#include "Config.h"
#include "Manager.h"
#include "MessageStream.h"
#include "Vector2Stream.h"
#include "CachingTextRenderer.h"

#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <functional>


#define DEFAULT_COLOR { 0x0, 0xFF, 0xFF, 0xFF }
#define DEFAULT_MAX_AGE 5.0

class ConsoleManager: public Manager
{
public:


public:

  int FontSize = 14;

  int Backlog = 1000;

  bool Visible = false;

  bool ExitRequested = false;

public:

  ConsoleManager();

  void AddMessage(const std::string& message, const Color& color = DEFAULT_COLOR, double max_age = DEFAULT_MAX_AGE);

  MessageStream GetStream(const Color& color = DEFAULT_COLOR, double max_age = DEFAULT_MAX_AGE)
  {
    return MessageStream(*this, color, max_age);
  }

  void Render(SDL_Renderer* renderer);

  void ToggleVisibility();

  void KeyPressed(SDL_Keysym& Keysym);

  void ExecuteCommand(const std::string& command);
  
  size_t GetTextCacheSize() const
  {
    return text_renderer.GetPixelCount();
  }

  double GetTextCacheHitRate()
  {
    return text_renderer.GetHitRate();
  }

private:

  float2 RenderPrompt(SDL_Renderer* renderer, float2 location);

  void RenderMessages(SDL_Renderer* renderer, const float2& location);
  
  CachingTextRenderer text_renderer;

  std::string Prompt = "> ";
  
  std::string Input = "";  
  size_t CursorPosition = 0;
  int HistoryCursorPosition = 0;

  struct Message
  {
    std::chrono::system_clock::time_point time;
    std::string text;
    SDL_Color color = DEFAULT_COLOR;
    double max_age = DEFAULT_MAX_AGE;
  };

  List<Message> Messages;

  List<std::string> EventsHistory;

  Map<std::string, std::function<void(const std::vector<std::string>& args)>>& GetCommands();

  Map<std::string, std::function<void(const std::vector<std::string>& args)>> Commands;

  Set<std::string> Suggestions;

  int SuggestionIndex = 0;

};
