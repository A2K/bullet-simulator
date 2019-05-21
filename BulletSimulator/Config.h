#pragma once

#include <cstdint>


class Config
{
public:

  static bool DestroyWallsOnCollision; // default: false
  
  static double TimeSpeedScale; // default: 1.0

  static bool ReverseTime; // default: false

  static size_t HistoryMaxBytes; // default: 1 * 1024 * 1024 (1 mb)
  
  static double HistoryMaxAge; // default: 10.0

  static size_t TextCacheMaxPixels; // default: 100000

  static double BulletSpeed; // default: 1000.0
  
  static double FireRate; // default: 10.0

  static uint32_t LastWallId; // default: 0

  static uint32_t LastBulletId; // default: 0

  static double BulletRadius; // default: 0.01

  static double DebugValue1; // default: 0.0

  static double DebugValue2; // default: 0.0

  static double DebugValue3; // default: 0.0

  static bool ShowDebugMarkers; // default: false

  static bool ShowDebugHistory; // default: false

  static double MovementSpeed; // default: 150.0

  static double BaseLineWidth; // default: 4.0

  static float RenderScale; // default: 1.0

  static size_t MaxPreciseBullets; // default: 25

  static bool ShowViewCollisions; // default: false

  static float TargetFPS; // default: 60.0

  static bool EnableParallelTextureGeneration; // default: true
  
  static bool ShowMouseLocation; // default: false

  static bool ShowViewLine; // default: true

  static double CameraInterpolationSpeed; // default: 5.0

  static double Acceleration; // default: 10.0

  static bool LogCollisions; // default: false
  
  static bool LogNetwork; // default: false

};
