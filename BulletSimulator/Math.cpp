#include "Common.h"

#include <cmath>


float Floor(const float& value)
{
  return std::floor(value);
}

double Floor(const double& value)
{
  return std::floorl(value);
}

float Ceil(const float& value)
{
  return std::ceilf(value);
}

double Ceil(const double& value)
{
  return std::ceill(value);
}
