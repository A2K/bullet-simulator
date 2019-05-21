#include "Common.h"

#include "LineSegment.h"

#include "Math2D.h"


float2 LineSegment::GetDirection() const
{
  return A.DirectionTo(B);
}

float2 LineSegment::GetNormal() const
{
  float2 direction = GetDirection();
  return float2(-direction.y, direction.x);
}

float2 LineSegment::Reflect(const float2& direction) const
{
  float2 wall_direction = GetDirection();
  float2 normal = float2(-wall_direction.y, wall_direction.x);
  return direction.Reflect(normal);
}

bool LineSegment::Intersect(const LineSegment& other, float2* point) const
{
  return Math2D::IntersectLineSegments(*this, other, point);
}

bool LineSegment::Intersect(const LineSegment& other, float2& point) const
{
  return Intersect(other, &point);
}
