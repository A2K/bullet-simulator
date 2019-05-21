#include "Common.h"

#include "MessageStream.h"

#include "ConsoleManager.h"


MessageStream::MessageStream(ConsoleManager& manager, const Color& color, double max_age) :
  Manager(manager),
  MessageColor(color),
  MaxAge(max_age)
{
  RefCount = new size_t(1);
  Stream = new std::stringstream();
}

MessageStream::MessageStream(const MessageStream& other) :
  RefCount(other.RefCount),
  Stream(other.Stream),
  Manager(other.Manager),
  MessageColor(other.MessageColor),
  MaxAge(other.MaxAge)
{
  if (RefCount) (*RefCount)++;
}

MessageStream::~MessageStream()
{
  DecrementRefCount();
}

MessageStream& MessageStream::operator=(const MessageStream& other)
{
  if (Stream != other.Stream)
  {
    DecrementRefCount();
    Stream = other.Stream;
  }

  MessageColor = other.MessageColor;
  MaxAge = other.MaxAge;

  if (other.RefCount)
  {
    (*(RefCount = other.RefCount))++;
  }

  return *this;
}

void MessageStream::DecrementRefCount()
{
  if (!RefCount) return;

  if (!(--(*RefCount)))
  {
    if (Stream)
    {
      Manager.AddMessage(reinterpret_cast<std::stringstream*>(Stream)->str(), MessageColor, MaxAge);
      delete Stream;
      Stream = nullptr;
    }

    delete RefCount;
    RefCount = nullptr;
  }
}
