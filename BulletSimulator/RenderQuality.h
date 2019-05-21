#pragma once


namespace Text
{
  enum RenderQuality
  {
    SOLID = 0,
    SHADED,
    BLENDED
  };

  const enum RenderQuality DefaultRenderQuality = RenderQuality::SHADED;
}
