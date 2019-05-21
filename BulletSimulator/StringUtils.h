#pragma once

#include "Types.h"
#include "Math.h"

#include <string>
#include <numeric>
#include <sstream>
#include <iterator>
#include <algorithm>


namespace String
{
  extern std::string& TrimLeft(std::string &s);

  extern std::string& TrimRight(std::string &s);

  extern std::string& Trim(std::string &s);

  extern std::string ToLower(const std::string& s);

  extern std::string ToUpper(const std::string& s);

  extern std::string Trimmed(std::string &orig);

  extern bool IsNumber(const std::string& s);
  
  extern std::string Format(float value, int precision = -1);

  extern std::string FormatPercentDynamic(float percent);

  extern std::string FormatBytes(size_t bytes);

  extern std::string FormatPercent(double fraction);

  template<template<class> typename T = List, typename S = std::string>
  T<S> Split(const S& s)
  {
    std::istringstream iss(s);

    return T<S>(
      (std::istream_iterator<std::string>(iss)),
      std::istream_iterator<std::string>());
  }

  template<typename ITER, typename S = std::string>
  S CommonPrefix(const ITER& begin, const ITER& end)
  {
    if (begin == end) return S();

    return std::reduce(begin, end, *begin,
      [](const S& a, const S& b)
    {
      size_t last_index = Min(a.length(), b.length());

      for (size_t j = 0; j < last_index; j++)
      {
        if (a[j] != b[j])
        {
          return a.substr(0, j);
        }
      }

      return a.substr(0, last_index);
    });
  };

  template<typename T, typename S = std::string>
  S CommonPrefix(const T& strings)
  {
    return CommonPrefix(strings.begin(), strings.end());
  }

  template<template<class> typename T = List, typename S = std::string>
  S Join(const T<S>& strings, const S& delimiter = ",")
  {
    std::stringstream s;
    size_t index = 0;
    for (const S& str : strings)
    {
      s << str;
      if (++index != strings.size())
        s << delimiter;
    }
    return s.str();
  }

  template<typename T, typename S = std::string>
  std::string Join(const T& strings, const S& delimiter = ",")
  {
    std::stringstream s;
    size_t index = 0;
    for (const auto& str : strings)
    {
      s << str;
      if (++index != strings.size())
        s << delimiter;
    }
    return s.str();
  }
}
