#include "Common.h"

#include "StringUtils.h"

#include <cctype>
#include <algorithm>


namespace String
{

  std::string& TrimLeft(std::string &s)
  {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
      return !std::isspace(ch);
    }));
    return s;
  }

  std::string& TrimRight(std::string &s)
  {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
      return !std::isspace(ch);
    }).base(), s.end());
    return s;
  }

  std::string& Trim(std::string &s)
  {
    return TrimRight(TrimLeft(s));
  }

  std::string Trimmed(std::string &orig)
  {
    std::string s = orig;
    TrimLeft(s);
    TrimRight(s);
    return s;
  }

  bool IsNumber(const std::string& s)
  {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
  }

  std::string ToLower(const std::string& s)
  {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
  }

  std::string ToUpper(const std::string& s)
  {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(), ::toupper);
    return res;
  }

  std::string FormatBytes(size_t bytes)
  {
    std::stringstream ss;

    int unit = 1024;

    if (bytes < unit)
    {
      return std::to_string(bytes) + " bytes";
    }

    int exp = (int)(std::log(bytes) / std::log(unit));

    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f %cB", bytes / std::pow(unit, exp), "KMGTPE"[exp - 1]);

    return std::string(buf);
  };

  std::string FormatPercent(double fraction)
  {
    char percent[64];
    snprintf(percent, sizeof(percent), "%.0f", fraction * 100.0);
    return std::string(percent);
  }

  std::string Format(float value, int precision)
  {
    char buf[64];

    if (precision >= 0)
    {
      char fmt[16];
      snprintf(fmt, sizeof(fmt), "%%.%df", precision);
      snprintf(buf, sizeof(buf), fmt, value);
    }
    else
    {
      if (value < 1)
      {
        char fmt[16];
        int symbols = int(std::ceilf(Max(0.0f, -std::log10(value))) + 1 + ((value < 0.01) ? 1 : 0));
        snprintf(fmt, sizeof(fmt), "%%.%df", symbols);
        snprintf(buf, sizeof(buf), fmt, value);
      }
      else if (value > 1 && value < 10)
      {
        snprintf(buf, sizeof(buf), "%.2f", value);
      }
      else if (value < 100)
      {
        snprintf(buf, sizeof(buf), "%.1f", value);
      }
      else
      {
        snprintf(buf, sizeof(buf), "%d", int(value));
      }
    }

    return std::string(buf);
  }

  std::string FormatPercentDynamic(float percent)
  {
    if (percent == 0) return "0";

    if (isnan(percent)) return "nan";
    if (isinf(percent)) return "inf";

    char buf[64];

    if (percent < 1)
    {
      char fmt[16];
      int symbols = int(Ceil(Max(0.0f, -std::log10(percent))) + 1 + ((percent < 0.01) ? 1 : 0));
      snprintf(fmt, sizeof(fmt), "%%.%df", symbols);
      snprintf(buf, sizeof(buf), fmt, percent);
    }
    else if (percent > 1 && percent < 10)
    {
      snprintf(buf, sizeof(buf), "%.2f", percent);
    }
    else if (percent < 100)
    {
      snprintf(buf, sizeof(buf), "%.1f", percent);
    }
    else
    {
      snprintf(buf, sizeof(buf), "%d", int(percent));
    }

    return std::string(buf);
  }
}