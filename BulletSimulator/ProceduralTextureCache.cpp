#include "Common.h"

#include "ProceduralTextureCache.h"


ProceduralTextureCache & ProceduralTextureCache::Get()
{
  static ProceduralTextureCache* instance = nullptr;
  return *(instance ? instance : instance = new ProceduralTextureCache());
}

ProceduralTextureCache::Generator ProceduralTextureCache::AddGenerator(const std::string & name, ProceduralTexture::Sampler generator)
{
  Generators.Add(name, { name, generator, false, 0 });

  return [this, name](SDL_Renderer* renderer, const int2& resolution)
  {
    return this->GetTexture(renderer, name, resolution);
  };
}

ProceduralTextureCache::Generator ProceduralTextureCache::AddGenerator(const std::string & name, uint8_t init_value, ProceduralTexture::Sampler generator)
{
  ProceduralTextureCache::GeneratorDesc& gen_desc = Generators.Add(name, { name, generator, false, 0 });

  return [this, gen_desc](SDL_Renderer* renderer, const int2& resolution)
  {
    return this->GetTexture(renderer, gen_desc, resolution);
  };
}

std::shared_ptr<ProceduralTexture> ProceduralTextureCache::GetTexture(SDL_Renderer * renderer, const ProceduralTextureCache::GeneratorDesc& generator, const int2 & resolution)
{
  std::shared_ptr<ProceduralTexture>* cached = Cache.Get({ generator.Name, resolution });

  if (cached) return *cached;
  
  TotalSize += resolution.x * resolution.y;

  return Cache.Add({ generator.Name, resolution },
    generator.Initialize 
    ? std::make_shared<ProceduralTexture>(resolution, generator.InitValue, renderer, generator.Function)
    : std::make_shared<ProceduralTexture>(resolution, renderer, generator.Function)
  );
}

std::shared_ptr<ProceduralTexture> ProceduralTextureCache::GetTexture(SDL_Renderer * renderer, const std::string & generator_name, const int2 & resolution)
{
  std::shared_ptr<ProceduralTexture>* cached = Cache.Get({ generator_name, resolution });

  if (cached) return *cached;

  auto generator = Generators.Get(generator_name);

  if (!generator) return nullptr;

  TotalSize += resolution.x * resolution.y;

  return Cache.Add({ generator_name, resolution }, 
    generator->Initialize 
    ? std::make_shared<ProceduralTexture>(resolution, generator->InitValue, renderer, generator->Function)
    : std::make_shared<ProceduralTexture>(resolution, renderer, generator->Function));
}
