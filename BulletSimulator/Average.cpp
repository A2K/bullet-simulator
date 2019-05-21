#include "Common.h"

#include "Average.h"


double Average::Add(size_t count, float duration)
{
  Count += count;
  Duration += duration;
  Total.Count += count;
  Total.Duration += duration;

  Rate = double(Total.Count) / (Total.Duration == 0 ? 1.0 : Total.Duration);

  while (Total.Duration > MaxDuration)
  {
    Total.Count -= Count.front();
    Count.pop_front();
    Total.Duration -= Duration.front();
    Duration.pop_front();
  }

  return Rate;
}
