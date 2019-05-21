#pragma once

#include "MessageStream.h"

#include <sstream>


extern MessageStream GetConsoleLogStream(const struct Color& color, double max_age);

class Logger
{
public:

  std::stringstream stream;

  virtual ~Logger();

  template <typename Value> 
  Logger& operator << (Value const& value)
  {
    stream << value;
    return *this;
  }
};

#ifdef _MSC_VER
#define __SEPARATOR__ '\\'
#else // _MSC_VER
#define __SEPARATOR__ '/'  
#endif // _MSC_VER

#define __FILENAME__ (strrchr(__FILE__, __SEPARATOR__) ? strrchr(__FILE__, __SEPARATOR__) + 1 : __FILE__)

#define LOG_COUT (Logger() << __FILENAME__ << ":" << __LINE__ << ": ")

#define LOG GetConsoleLogStream({ 0, 0xFF, 0xFF, 0xFF }, 5.0)
#define LOG__(color, max_age) GetConsoleLogStream(color, max_age)
#define LOG_(color) LOG__(color, 5.0)

#define COLOR_GREY(L) { L, L, L, 0xFF }
#define COLOR(R, G, B) { R, G, B, 0xFF }

#define COLOR_LOG_DEFAULT COLOR(0x0, 0xFF, 0xFF)
#define COLOR_LOG_ERROR COLOR(0xFF, 0x62, 0x4C)
#define COLOR_LOG_WARNING COLOR(0xFF, 0xA3, 0x72)
#define COLOR_LOG_SUCCESS COLOR(0x08, 0xFF, 0x92)
#define COLOR_LOG_DEBUG COLOR_GREY(0xAD)

#define LOG_ERROR LOG__(COLOR_LOG_ERROR, 10.0)
#define LOG_WARNING LOG__(COLOR_LOG_WARNING, 7.5)
#define LOG_SUCCESS LOG_(COLOR_LOG_SUCCESS)
#define LOG_DEBUG LOG__(COLOR_LOG_DEBUG, 1.0)
