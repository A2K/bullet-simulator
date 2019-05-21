#pragma once


#pragma pack(push, 4)

namespace Protocol
{

  enum PacketType
  {
    ADD = 0,
    UPDATE,
    REMOVE,
    SYNC,
    WORLD_SYNC
  };

  enum EntityType
  {
    BULLET = 0,
    WALL
  };

}

#pragma pack(pop)
