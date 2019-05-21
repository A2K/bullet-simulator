#pragma once

#include "Map.h"
#include "Types.h"
#include "ProceduralTexture.h"
#include "Logger.h"


class ProceduralTextureCache
{
public:

  static ProceduralTextureCache& Get();

  typedef std::function<std::shared_ptr<ProceduralTexture>(SDL_Renderer*, const int2&)> Generator;

  Generator AddGenerator(const std::string& name, ProceduralTexture::Sampler generator);

  Generator AddGenerator(const std::string& name, uint8_t init_value, ProceduralTexture::Sampler generator);


  std::shared_ptr<ProceduralTexture> GetTexture(SDL_Renderer* renderer, const std::string& generator_name, const int2& resolution);

  size_t TotalSize = 0;

private:

  struct GeneratorDesc 
  {
    std::string Name;
    ProceduralTexture::Sampler Function;
    bool Initialize = false;
    uint8_t InitValue;

    GeneratorDesc() {}

    GeneratorDesc(const std::string& name, ProceduralTexture::Sampler function, bool initialize, uint8_t init_value):
      Name(name), Function(function), Initialize(initialize), InitValue(init_value)
    {}
  };

  std::shared_ptr<ProceduralTexture> GetTexture(SDL_Renderer * renderer, const GeneratorDesc& generator, const int2 & resolution);

  Map<std::string, GeneratorDesc> Generators;

  Map<std::pair<std::string, int2>, std::shared_ptr<ProceduralTexture>> Cache;
  
};
