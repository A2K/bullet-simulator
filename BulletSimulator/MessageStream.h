#pragma once

#include "Color.h"

#include <sstream>


class ConsoleManager;

class MessageStream
{
  size_t* RefCount = nullptr;

  std::stringstream* Stream = nullptr;

  ConsoleManager& Manager;

  Color MessageColor;

  double MaxAge;

public:

  MessageStream(ConsoleManager& manager, const Color& color = { 0x0, 0xFF, 0xFF, 0xFF }, double max_age = 5.0);

  MessageStream(const MessageStream& other);

  ~MessageStream();
  
  MessageStream& operator=(const MessageStream& other);

  template <typename T> 
  inline std::ostream& operator << (T const& value)
  {
    return (*Stream) << value;
  }

private:

  void DecrementRefCount();

};
