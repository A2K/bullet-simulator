#pragma once

#include "List.h"


class Average
{
public:

  double MaxDuration = 1.0; // seconds

  double Rate = 0.0;

public:

  explicit Average(double duration = 1.0):
    MaxDuration(duration) {}

  double Add(size_t count, float duration);

  struct
  {
    uint64_t Count = 0;
    double Duration = 0;
  } Total;

private:

  List<size_t> Count;
  List<float> Duration;


};
