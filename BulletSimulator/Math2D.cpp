#include "Common.h"

#include "Math2D.h"

#include "Wall.h"
#include "Math.h"
#include "Config.h"
#include "Logger.h"
#include "LineSegment.h"


bool Math2D::IntersectLineSegments(
  const float2& p0, const float2& p1,
  const float2& p2, const float2& p3,
  float2* result)
{
  const float2 s1 = p1 - p0;
  const float2 s2 = p3 - p2;

  const float denom = (-s2.x * s1.y + s1.x * s2.y);

  const float2 dp02 = p0 - p2;

  const float s = (-s1.y * dp02.x + s1.x * dp02.y) / denom;

  if (!(s >= 0 && s <= 1)) return false;
  
  const float t = (s2.x * dp02.y - s2.y * dp02.x) / denom;

  if (!(t >= 0 && t <= 1)) return false;
  
  if (result) (*result) = p0 + s1 * t;

  return true;
}

bool Math2D::IntersectLineSegments(const LineSegment& a, const LineSegment& b, float2* point)
{
  return Math2D::IntersectLineSegments(a.A, a.B, b.A, b.B, point);
}

float2 Math2D::OffsetIntersectionRadius(const LineSegment& segment, const float2& point, const float2& direction, float radius)
{
  float forward_distance = radius / std::sin(std::acos(segment.GetDirection().Dot(direction)));
  return point - direction * forward_distance;
}

bool Math2D::IntersectLineSegmentsRadius(const LineSegment& a, const LineSegment& b, float radius, float2* point)
{
  float2 intersection;
  if (Math2D::IntersectLineSegments(a, b, &intersection))
  {
    intersection = OffsetIntersectionRadius(a, intersection, b.GetDirection(), radius);
    if (point)
    {
      *point = intersection;
    }
    return true;
  }

  return false;
}

float Math2D::DistanceFromPointToLine(const LineSegment& line, const float2& point)
{
  return ((line.B.y - line.A.y) * point.x 
           - (line.B.x - line.A.x) * point.y
           + line.B.x * line.A.y 
           - line.B.y * line.A.x)
         / line.Length();
}

bool Math2D::RayLineIntersection(const float2& point, const float2& direction, const LineSegment& line, float2* intersection_point, float* sin_angle_out)
{
  float distance_to_line = Math2D::DistanceFromPointToLine(line, point);

  if (distance_to_line == 0)
  {
    if (intersection_point) *intersection_point = point;
    if (sin_angle_out) *sin_angle_out = 0;
    return true;
  }

  float2 vector_to_line = line.GetNormal() * distance_to_line;

  if (vector_to_line.Dot(direction) < 0) return false;

  if (!sin_angle_out) return true;

  float cos_angle = line.GetDirection().Dot(direction);

  float sin_angle = sin(acos(cos_angle));

  if (sin_angle_out) *sin_angle_out = sin_angle;

  if (!intersection_point) return true;

  float distance = Abs(distance_to_line / sin_angle);

  *intersection_point = point + direction * distance;

  return true;
}

bool Math2D::RayLineIntersection(const float2& point, const float2& direction, const LineSegment& line, float2& intersection_point)
{
  return Math2D::RayLineIntersection(point, direction, line, &intersection_point);
}

bool Math2D::PointLineIntersection(const LineSegment& line, const float2& point, const float2& direction, float2* intersection_point, float* intersection_distance)
{
  float distance_to_line = Math2D::DistanceFromPointToLine(line, point);

  if (intersection_distance) *intersection_distance = distance_to_line;

  float2 vector_to_line = line.GetNormal() * distance_to_line;

  if (vector_to_line.Dot(direction) < 0) return false;

  distance_to_line = Abs(distance_to_line);  

  if (Abs(distance_to_line) <= Config::BulletRadius)
  {
    if (isnan(point.x) || isnan(point.y))
    {
      LOG_ERROR << "Math2D::PointLineIntersection isnan(point.x)";
      return false;
    }
    if (Max(point.DistanceTo(line.A), point.DistanceTo(line.B)) <= line.Length())
    {
      if (intersection_point) *intersection_point = point;
      return true;
    }
  }

  float cos_angle = Clamp(line.GetDirection().Dot(direction), -1.0f, 1.0f);

  float sin_angle = sin(acos(cos_angle));

  float abs_sin_angle = Abs(sin_angle);

  float distance = 0.0f;

  if (abs_sin_angle == 0)
  {
    return false;
  }

  distance = distance_to_line / abs_sin_angle;

  if (distance_to_line > Config::BulletRadius)
  {
    distance = distance - Config::BulletRadius / sin_angle;
  }

  float2 location = point + direction * distance;

  if (intersection_point) *intersection_point = location;

  if (isnan(location.x))
  {
    LOG_ERROR << "Math2D::PointLineIntersection isnan(location.x)";
    return false;
  }

  if (Max(location.DistanceTo(line.A), location.DistanceTo(line.B)) > line.Length())
    return false;

  {
    float distance_to_line = Math2D::DistanceFromPointToLine(line, location);

    float2 vector_to_line = line.GetNormal() * distance_to_line;
    
    float2 incident_point = location + vector_to_line;

    float max_end_distance = Max(incident_point.DistanceTo(line.A), incident_point.DistanceTo(line.B));

    if (max_end_distance > line.Length()) return false;
  }  

  return true;
}

bool Math2D::PointLineIntersection(const LineSegment& line, const float2& point, const float2& direction, float2& intersection_point, float& intersection_distance)
{
  return PointLineIntersection(line, point, direction, &intersection_point, &intersection_distance);
}

bool Math2D::PointLineIntersection(const LineSegment& line, const float2& point, const float2& direction, float2& intersection_point)
{
  return PointLineIntersection(line, point, direction, &intersection_point);
}

bool Math2D::CirclePointIntersection(const float2& point, const float2& origin, const float2& direction, const float radius, const float range,
  float2& location, float2& normal)
{
  LineSegment view_line = { origin, origin + direction * range };

  float distance = Math2D::DistanceFromPointToLine(view_line, point);

  if (distance == 0)
  {
    if (origin.DirectionTo(point).Dot(direction) < 0) return false;
    normal = point.DirectionTo(origin);
    float distance = point.DistanceTo(origin);
    if (distance < radius)
      location = origin;
    else
      location = point - direction * (distance - radius);
    return true;
  }

  if (Abs(distance) > radius) return false;

  float2 v = view_line.GetNormal() * distance;

  float collision_distance = sqrt(pow(radius, 2.0f) - pow(v.Length(), 2.0f));

  float2 ddir = origin.DirectionTo(point + v);

  float2 offset = v - ddir * collision_distance;

  location = point + offset;

  normal = point.DirectionTo(location);

  return true;
}

bool Math2D::CircleLineIntersection(const LineSegment& segment, const float2& origin, const float2& direction, const float radius, const float range, 
  float2& location, float2& normal)
{
  float distance;
  bool hit = Math2D::PointLineIntersection(segment, origin, direction, location, distance);

  if (hit && isnan(location.x))
  {
    LOG_ERROR << "Math2D::CircleLineIntersection isnan(location.x)";
    return false;
  }

  if (!hit && distance <= Config::BulletRadius && 
      Max(origin.DistanceTo(segment.A), origin.DistanceTo(segment.B)) <= segment.Length())
  {
    return false;
  }

  float2 location_tmp;
  float2 normal_tmp;
  
  auto check_end = [&](const float2& point) -> bool
  {
    if (!Math2D::CirclePointIntersection(point, origin, direction, radius, range, location_tmp, normal_tmp))
      return false;
    
    if (hit && origin.DistanceTo(location_tmp) > origin.DistanceTo(location))
      return false;
      
    if (origin.DirectionTo(location_tmp).Dot(direction) <= 0)
      return false;
        
    location = location_tmp;
    normal = normal_tmp;

    return hit = true;
  };

  bool hit_end_A = check_end(segment.A);
  bool hit_end_B = check_end(segment.B);

  if (hit_end_A || hit_end_B)
  {
    return true;
  }

  if (hit)
  {
    normal = -segment.GetNormal() * Sign(distance);
    return true;
  }

  return false;
}

float Math2D::GetAngleRadians(const float2& vector)
{
  return atan2(vector.y, vector.x);
}

float Math2D::GetAngleRadians(const LineSegment& line)
{
  return Math2D::GetAngleRadians(line.GetDirection());
}
