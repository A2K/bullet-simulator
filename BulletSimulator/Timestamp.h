#pragma once

#include <cstdint>


#pragma pack(push, 4)

template<typename T = uint64_t, int Scale = 9, typename F = double>
struct Timestamp
{
  const T Infinity = std::numeric_limits<T>::max();
  T Value = std::numeric_limits<T>::max();

  Timestamp() {}
  
  Timestamp(T microseconds) :
    Value(microseconds)
  {}

  Timestamp(F time)
  {
    SetTime(time);
  }

  void SetTime(F time)
  {
    if (isinf(time))
    {
      Value = Infinity;
    }
    else
    {
      Value = T(std::llroundl(time * pow(10, Scale)) & Infinity);
      if (Value == Infinity)
      {
        Value = 0;
      }
    }
  }

  F GetTime(F world_time) const
  {
    if (IsInifinity())
    {
      return std::numeric_limits<F>::max();
    }
    else
    {
      static const F period = F(Infinity - 1) / pow(10, Scale);

      F time = F(Value) / pow(10, Scale);

      while (world_time > time + period * 0.5)
      {
        time += period;
      }

      while (world_time < time - period * 0.5)
      {
        time -= period;
      }

      return time;
    }
  }

  void SetInfinity()
  {
    Value = Infinity;
  }

  bool IsInifinity() const
  {
    return Value == Infinity;
  }

  operator F() const
  {
    return GetTime(World::Get().CurrentTime);
  }

  Timestamp& operator=(const F& value)
  {
    this->SetTime(value);
    return *this;
  }

  Timestamp& operator=(const Timestamp& other)
  {
    this->Value = other.Value;
    return *this;
  }
};

#pragma pack(pop)
