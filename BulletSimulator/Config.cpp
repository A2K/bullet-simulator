#include "Config.h"

#include "Common.h"


bool Config::DestroyWallsOnCollision = true;

double Config::TimeSpeedScale = 1.0;

bool Config::ReverseTime = false;

size_t Config::HistoryMaxBytes = 100 * 1024 * 1024;

double Config::HistoryMaxAge = 30.0;

size_t Config::TextCacheMaxPixels = size_t(9.99 * 1024 * 1024 / 4);

double Config::BulletSpeed = 500.0;

double Config::FireRate = 10.0;

uint32_t Config::LastWallId = 0;

uint32_t Config::LastBulletId = 0;

double Config::BulletRadius = 3;

double Config::DebugValue1 = 0.0;

double Config::DebugValue2 = 0.0;

double Config::DebugValue3 = 0.0;

bool Config::ShowDebugMarkers = false;

bool Config::ShowDebugHistory = false;

double Config::MovementSpeed = 300.0;

double Config::BaseLineWidth = 4.0;

float Config::RenderScale = 1.0f;

size_t Config::MaxPreciseBullets = 25;

bool Config::ShowViewCollisions = false;

float Config::TargetFPS = 60.0f;

bool Config::EnableParallelTextureGeneration = true;

bool Config::ShowMouseLocation = false;

bool Config::ShowViewLine = true;

double Config::CameraInterpolationSpeed = 5.0;

double Config::Acceleration = 5.0;

bool Config::LogCollisions = false;

bool Config::LogNetwork = false;
