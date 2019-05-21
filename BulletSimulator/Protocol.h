#pragma once

#include "Wall.h"
#include "Bullet.h"
#include "ProtocolEnums.h"

#include <memory>
#include <functional>


#pragma pack(push, 4)

namespace Protocol
{
  struct BulletDescriptor
  {
    static const uint8_t EntityType = Protocol::EntityType::BULLET;

    Bullet::id_t ID = 0;
    Timestamp<> Time;
    float Speed = 0;
    float2 Location;
    float2 Direction;
    float Lifetime = 0;

    BulletDescriptor() {}

    BulletDescriptor(const Bullet& bullet, double time):
      ID(bullet.ID),
      Time(time),
      Speed(bullet.Speed),
      Location(bullet.GetLocation(time)),
      Direction(bullet.Direction),
      Lifetime(bullet.Lifetime - (time - bullet.Time))
    {
    }

    Bullet ToBullet(double time) const
    {
      return Bullet(ID, Location, Direction, Speed, Time.GetTime(time), Lifetime);
    }

    bool operator<(const BulletDescriptor& other) const
    {
      return ID < other.ID;
    }
  };

  namespace Packets
  {
    struct Sync
    {
      uint8_t Type = PacketType::SYNC;
    };

    template<typename T>
    struct Update
    {
      uint8_t Type = 0;
      uint8_t EntityType = 0;
      uint32_t Count = 0;

    private:

      Update(enum PacketType type, size_t count) :
        Type(type), EntityType(T::EntityType), Count(count)
      {}

    public:

      static std::shared_ptr<Update> Make(enum PacketType type, size_t count, size_t& byte_count)
      {
        byte_count = sizeof(Update) + sizeof(T) * count;

        std::shared_ptr<uint8_t> bytes = std::shared_ptr<uint8_t>(new uint8_t[byte_count], [](uint8_t* ptr) { delete[] ptr; });

        new (bytes.get()) Update<T>(type, count);

        return std::reinterpret_pointer_cast<Update<T>>(bytes);
      }

      struct
      {
        T& operator[](size_t i) const
        {
          return *(const_cast<T*>(reinterpret_cast<const T*>(this) + i));
        }
      } Data;

      void ForEach(std::function<void(T&)> func)
      {
        for (size_t i = 0; i < Count; ++i)
        {
          func(Data[i]);
        }
      }

      void ForEach(std::function<void(T&, size_t)> func)
      {
        for (size_t i = 0; i < Count; ++i)
        {
          func(Data[i], i);
        }
      }

      void ForEach(std::function<void(const T&)> func) const
      {
        for (size_t i = 0; i < Count; ++i)
        {
          func(Data[i]);
        }
      }

      void ForEach(std::function<void(const T&, size_t)> func) const
      {
        for (size_t i = 0; i < Count; ++i)
        {
          func(Data[i], i);
        }
      }
    };

    struct WorldSync
    {
      uint8_t Type = PacketType::WORLD_SYNC;

      double Time = 0.0;

      struct
      {
        uint32_t WallCount = 0;
        uint32_t BulletCount = 0;
      } Header;

    private:

      WorldSync(size_t wall_count, size_t bullet_count):
        Header({ uint32_t(wall_count), uint32_t(bullet_count) })
      {}
      
    public:

      static std::shared_ptr<WorldSync> Make(size_t wall_count, size_t bullet_count, size_t& byte_count);

      struct
      {
        Wall& operator[](size_t i) const 
        {
          return *(const_cast<Wall*>(reinterpret_cast<const Wall*>(this) + i));
        }
      } Walls;

      struct
      {
        BulletDescriptor& operator[](size_t i) const
        {
          static const WorldSync s(0, 0);
          static const size_t offset = reinterpret_cast<const uint8_t*>(&s.Bullets) - reinterpret_cast<const uint8_t*>(&s);

          const WorldSync* world = reinterpret_cast<const WorldSync*>(reinterpret_cast<const uint8_t*>(this) - offset);
          
          const Wall* walls = reinterpret_cast<const Wall*>(this);

          const BulletDescriptor* bullets = reinterpret_cast<const BulletDescriptor*>(walls + world->Header.WallCount);

          return *const_cast<BulletDescriptor*>(bullets + i);
        }        
      } Bullets;

    };

  }

}
#pragma pack(pop)
