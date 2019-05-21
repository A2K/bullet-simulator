#include "Common.h"

#include "Math.h"
#include "World.h"
#include "Logger.h"
#include "OverlayManager.h"
#include "ConsoleManager.h"

#include <SDL.h>


OverlayManager::OverlayManager()
{
}


OverlayManager::~OverlayManager()
{
}

void OverlayManager::AddLabel(const float2& location, const std::string& text, float size, const Color& color)
{
  this->Labels += std::make_shared<TextLabel>(location, text, size, color);
}

void OverlayManager::AddLabel(const float2& location, std::function<std::string()> text_delegate, float size, const Color& color)
{
  this->Labels += std::make_shared<DelegateLabel>(location, text_delegate, size, color);
}

void OverlayManager::AddLabel(const float2& location, std::function<std::string(Color&)> text_delegate, float size)
{
  this->Labels += std::make_shared<ColorDelegateLabel>(location, text_delegate, size);
}

void OverlayManager::AddLabel(const float2& location, std::function<void(std::stringstream& stream)> stream_delegate, float size, const Color& color)
{
  this->Labels += std::make_shared<DelegateLabel>(location, [stream_delegate]() -> std::string
  {
    std::stringstream stream;
    stream_delegate(stream);
    return stream.str();
  }, size, color);
}

void OverlayManager::AddLabel(const float2& location, std::function<void(std::stringstream& stream, Color& color)> stream_delegate, float size)
{
  this->Labels += std::make_shared<ColorDelegateLabel>(location, [stream_delegate](Color& color) -> std::string
  {
    std::stringstream stream;
    stream_delegate(stream, color);
    return stream.str();
  }, size);
}

void OverlayManager::Render(SDL_Renderer* renderer)
{
  for (std::shared_ptr<LabelBase> label : Labels)
  {
    std::string text = label->GetText();

    if (!label->RenderData.Texture || label->RenderData.LastRenderedText != text)
    {
      float2 text_size;
      label->RenderData.Texture = Text.GetTexture(renderer, text, Round(label->Size), Color::WHITE, text_size);
      label->RenderData.Rects.SetSize(text_size);
      label->RenderData.Rects.SetLocation(label->Location);
    }

    SDL_Texture* texture = label->RenderData.Texture.get();
    SDL_SetTextureColorMod(texture, label->TextColor.r, label->TextColor.g, label->TextColor.b);
    SDL_SetTextureAlphaMod(texture, label->TextColor.a);
    SDL_RenderCopy(renderer, texture, &label->RenderData.Rects.Src, &label->RenderData.Rects.Dst);
  }
}
