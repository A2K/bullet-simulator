#include "Common.h"

#include "Color.h"
#include "World.h"
#include "Logger.h"
#include "MessageStream.h"
#include "ConsoleManager.h"

#ifdef _MSVC_LANG
#include <Windows.h>
#else
#include <iostream>
#endif


Logger::~Logger()
{
  stream << "\n";
#ifdef _MSVC_LANG
  OutputDebugString(stream.str().c_str());
#else
  std::cout << stream.str();
#endif
}

MessageStream GetConsoleLogStream(const Color& color, double max_age)
{
  return World::Get().GetManager<ConsoleManager>()->GetStream(color, max_age);
}
