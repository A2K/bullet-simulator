#pragma once

#include "Set.h"
#include "Types.h"
#include "Config.h"

#include <SDL.h>

#include <memory>
#include <chrono>
#include <cstdint>


namespace Text
{

  class TextureCache
  {
  public:

    struct Key
    {
      std::string Text;
      int Size;
      SDL_Color Color;
      Text::RenderQuality Quality;

      bool operator<(const Key& other) const
      {
        if (Size != other.Size) return Size < other.Size;
        int color_cmp = memcmp(&Color, &other.Color, sizeof(SDL_Color));
        if (color_cmp != 0) return color_cmp < 0;
        return Text < other.Text;
      }

      bool operator==(const Key& other) const
      {
        return Text == other.Text
          && Size == other.Size
          && memcmp(&Color, &other.Color, sizeof(SDL_Color)) == 0
          && Quality == other.Quality;
      }
    };

    std::shared_ptr<SDL_Texture> Get(const Key& key, float2& size)
    {
      auto iter = Cache.find(key);

      if (iter == Cache.end())
      {
        Stats.Miss();
        return std::shared_ptr<SDL_Texture>();
      }
      else
      {
        Stats.Hit();
      }

      iter->second.Time = std::chrono::system_clock::now();

      size = iter->second.Size;

      return iter->second.Texture;
    }

    void Set(const Key& key, std::shared_ptr<SDL_Texture> texture, float2 size)
    {
      KeysAndValues.insert({ key, (Cache[key] = Value(texture, size)) });
      PixelCount += size.x * size.y;
      Cleanup();
    }

    void Set(const Key& key, std::shared_ptr<SDL_Texture> texture)
    {
      int width, height;
      SDL_QueryTexture(texture.get(), nullptr, nullptr, &width, &height);
      Set(key, texture, { width, height });
    }

    void Forget(const Key& key)
    {
      auto iter = Cache.find(key);

      if (iter == Cache.end()) return;

      float2 size = iter->second.Size;

      PixelCount -= size.x * size.y;

      Cache.erase(iter);
    }

    void Cleanup()
    {
      while (PixelCount > Config::TextCacheMaxPixels && KeysAndValues.size())
      {
        auto iter = KeysAndValues.begin();
        {
          auto cache_iter = Cache.find(iter->Key);

          if (cache_iter != Cache.end())
          {
            float2 size = cache_iter->second.Size;

            PixelCount -= size.x * size.y;

            Cache.erase(cache_iter);
          }
        }
        KeysAndValues.erase(iter);
      }
    }

    size_t GetCachedPixelCount() const
    {
      return PixelCount;
    }

    struct 
    {
      size_t Hits = 0;
      size_t Misses = 0;

      void Hit()
      {
        Hits++;
      }

      void Miss()
      {
        Misses++;
      }

      double GetHitRate() const
      {
        if (!Hits && !Misses) return 0.0;
        return double(Hits) / (double(Hits) + double(Misses));
      }
    } Stats;

  private:

    size_t PixelCount = 0;

    struct Value
    {
      std::shared_ptr<SDL_Texture> Texture;

      float2 Size;

      std::chrono::system_clock::time_point Time = std::chrono::system_clock::now();

      Value() {}

      Value(std::shared_ptr<SDL_Texture> texture, float2 size) :
        Texture(texture),
        Size(size)
      {}
    };

    struct KeyValue
    {
      Key Key;
      Value& Value;

      bool operator<(const KeyValue& other) const
      {
        return (Value.Time == other.Value.Time)
          ? (Key < other.Key)
          : (Value.Time < other.Value.Time);
      }
    };

    Map<Key, Value> Cache;

    ::Set<KeyValue> KeysAndValues;

  };
}
