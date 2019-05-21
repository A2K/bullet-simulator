#pragma once

#include "Types.h"


struct LineSegment;

namespace Math2D
{
  float GetAngleRadians(const float2& vector);
  float GetAngleRadians(const LineSegment& line);
    
  bool IntersectLineSegments(
    const float2& p0, const float2& p1,
    const float2& p2, const float2& p3,
    float2 *intersection_point = nullptr);

  bool IntersectLineSegments(const LineSegment& a, const LineSegment& b, float2* point = nullptr);

  bool IntersectLineSegmentsRadius(const LineSegment& a, const LineSegment& b, float radius, float2* point = nullptr);

  float2 OffsetIntersectionRadius(const LineSegment& segment, const float2& point, const float2& direction, float radius);

  float DistanceFromPointToLine(const LineSegment& line, const float2& point);

  bool RayLineIntersection(const float2& point, const float2& direction, const LineSegment& line, float2* intersection_point = nullptr, float* sin_angle_out = nullptr);
  bool RayLineIntersection(const float2& point, const float2& direction, const LineSegment& line, float2& intersection_point);
  
  bool PointLineIntersection(const LineSegment& line, const float2& point, const float2& direction, float2* intersection_point = nullptr, float* intersection_distance = nullptr);  
  bool PointLineIntersection(const LineSegment& line, const float2& point, const float2& direction, float2& intersection_point, float& intersection_distance);
  bool PointLineIntersection(const LineSegment& line, const float2& point, const float2& direction, float2& intersection_point);
  
  bool CirclePointIntersection(const float2& point, const float2& origin, const float2& direction, const float radius, const float range, float2& location, float2& normal);

  bool CircleLineIntersection(const LineSegment& segment, const float2& origin, const float2& direction, const float radius, const float range, float2& point, float2& normal);

}
