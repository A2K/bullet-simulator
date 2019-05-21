#pragma once

#include "Rect.h"
#include "Color.h"
#include "Config.h"
#include "Manager.h"

#include "CachingTextRenderer.h"

#include <functional>


class OverlayManager: public Manager
{
public:

  struct LabelBase
  {
    float2 Location;
    float Size;
    Color TextColor;

    LabelBase(const float2& location, const float size, const Color& text_color) :
      Location(location), Size(size), TextColor(text_color)
    {}

    virtual inline std::string GetText()
    {
      return std::string();
    }

    struct {
      struct
      {
        Rect Src = { { 0, 0 }, { 0, 0 } };
        Rect Dst = { { 0, 0 }, { 0, 0 } };
        void SetSize(int width, int height)
        {
          Dst.size() = Src.size() = { width, height };
        }
        template<typename T>
        void SetSize(const Vector2<T>& size)
        {
          Dst.size() = Src.size() = size;
        }
        void SetLocation(int x, int y)
        {
          Dst.origin() = { x, y };
        }
        template<typename T>
        void SetLocation(const Vector2<T>& xy)
        {
          Dst.origin() = xy;
        }
      } Rects;
      std::shared_ptr<SDL_Texture> Texture;
      std::string LastRenderedText;
    } RenderData;
  };

  struct TextLabel: public LabelBase
  {
    std::string Text;

    TextLabel(const float2& location, const std::string& text,
          const float size, const Color& text_color):
      LabelBase(location, size, text_color),
      Text(text)
    {}

    virtual inline std::string GetText() override
    {
      return Text;
    }
  };

  struct DelegateLabel : public LabelBase
  {
    typedef std::function<std::string()> Delegate;

    DelegateLabel(const float2& location, Delegate delegate,
      const float size, const Color& text_color) :
      LabelBase(location, size, text_color),
      TextDelegate(delegate)
    {}

    Delegate TextDelegate;

    virtual inline std::string GetText() override
    {
      if (TextDelegate) return TextDelegate();
      return LabelBase::GetText();
    }
  };

  struct ColorDelegateLabel : public LabelBase
  {
    typedef std::function<std::string(Color&)> Delegate;

    ColorDelegateLabel(const float2& location, Delegate delegate, const float size) :
      LabelBase(location, size, Color::WHITE),
      TextDelegate(delegate)
    {}

    Delegate TextDelegate;

    virtual inline std::string GetText() override
    {
      if (TextDelegate) return TextDelegate(TextColor);
      return LabelBase::GetText();
    }
  };

  OverlayManager();

  ~OverlayManager();

  void AddLabel(const float2& location, const std::string& text, float size, const Color& color);

  void AddLabel(const float2& location, std::function<std::string()> text_delegate, float size, const Color& color);

  void AddLabel(const float2& location, std::function<std::string(Color&)> text_delegate, float size);

  void AddLabel(const float2& location, std::function<void(std::stringstream& stream)> stream_delegate, float size, const Color& color);
  
  void AddLabel(const float2& location, std::function<void(std::stringstream& stream, Color& color)> stream_delegate, float size);

  void Render(struct SDL_Renderer* renderer);

  inline size_t GetTextCacheSize() const
  {
    return Text.GetPixelCount();
  }

  inline double GetTextCacheHitRate() const
  {
    return Text.GetHitRate();
  }

  inline size_t GetTextCacheMaxSize() const
  {
    return Config::TextCacheMaxPixels;
  }

private: 

  List<std::shared_ptr<LabelBase>> Labels;

  CachingTextRenderer Text;

};

