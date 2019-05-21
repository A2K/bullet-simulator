#include "Common.h"
#include "Protocol.h"


std::shared_ptr<Protocol::Packets::WorldSync> Protocol::Packets::WorldSync::Make(size_t wall_count, size_t bullet_count, size_t & byte_count)
{
  byte_count = sizeof(WorldSync) + sizeof(Wall) * wall_count + sizeof(Bullet) * bullet_count;

  std::shared_ptr<uint8_t> bytes = std::shared_ptr<uint8_t>(new uint8_t[byte_count], [](uint8_t* ptr) { delete[] ptr; });

  new (bytes.get()) WorldSync(wall_count, bullet_count);

  return std::reinterpret_pointer_cast<WorldSync>(bytes);
}
