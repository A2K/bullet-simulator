#include "Common.h"

#include "NetworkPeer.h"

#include "World.h"
#include "Config.h"
#include "Logger.h"
#include "Protocol.h"
#include "BulletManager.h"

#include <enet/enet.h>


std::string NetworkPeer::AddressToString(uint32_t ipAddress)
{
  char ipAddr[16];
  snprintf(ipAddr, sizeof ipAddr, "%u.%u.%u.%u",
    (ipAddress & 0x000000ff),
    (ipAddress & 0x0000ff00) >> 8,
    (ipAddress & 0x00ff0000) >> 16,
    (ipAddress & 0xff000000) >> 24);
  return ipAddr;
}

NetworkPeer::~NetworkPeer()
{
  if (ListenThread)
  {
    Done = true;
    ListenThread->join();
  }

  if (ENet.Peer)
  {
    enet_peer_disconnect(ENet.Peer, 0);
    ENet.Peer = nullptr;
  }

  if (ENet.Host)
  {
    enet_host_destroy(ENet.Host);
    ENet.Host = nullptr;
  }
}

void NetworkPeer::Listen(uint32_t timeout_ms)
{
  Done = false;

  ENetEvent event;

  int code = 0;
  while ((code = enet_host_service(ENet.Host, &event, timeout_ms)) >= 0)
  {
    if (Done)
    {
      LOG << Name << ": stop requested";
      break;
    }

    if (code > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
      {
        LOG_SUCCESS << Name << ": connected " << AddressToString(event.peer->address.host) << ":" << event.peer->address.port;

        std::stringstream s;
        s << AddressToString(event.peer->address.host)
          << ":"
          << event.peer->address.port;

        std::string str = s.str();

        event.peer->data = new char[str.size() + 1];
        memcpy(event.peer->data, str.c_str(), str.size() + 1);

        for (auto handler : ConnectHandlers)
        {
          handler(&event);
        }
      }
      break;
      case ENET_EVENT_TYPE_RECEIVE:
      {
        if (Config::LogNetwork)
          LOG_DEBUG << Name << ": received " << event.packet->dataLength << " bytes"
            << " from " << reinterpret_cast<char*>(event.peer->data)
            << " on channel " << int(event.channelID);

        if (event.packet->dataLength > 0)
        {
          auto iter = PacketHandlers.find(event.packet->data[0]);
          if (iter != PacketHandlers.end())
          {
            for (auto handler : iter->second)
            {
              handler(event.peer, event.packet->data, event.packet->dataLength);
            }
          }
        }

        enet_packet_destroy(event.packet);
      }
      break;
      case ENET_EVENT_TYPE_DISCONNECT:
      {
        if (event.peer)
        {
          LOG << Name << ": disconnected " << reinterpret_cast<char*>(event.peer->data);
        }
        else
        {
          LOG << Name << ": connection lost";
        }

        for (auto handler : DisconnectHandlers)
        {
          handler(&event);
        }

        if (event.peer->data)
        {
          delete[] reinterpret_cast<uint8_t*>(event.peer->data);
          event.peer->data = nullptr;
        }
      }
      break;
      }
    }
  }

  Done = true;

  enet_host_destroy(ENet.Host);

  ENet.Host = nullptr;

  LOG << Name << ": stopped";
}

void NetworkPeer::Fire(const Bullet& bullet)
{

}

void NetworkPeer::AddWall(const Wall& wall)
{

}

void NetworkPeer::Send(VoidPointer peer, const void* data, size_t byte_count)
{
  ENetPacket* packet = enet_packet_create(data, byte_count, ENET_PACKET_FLAG_RELIABLE);

  if (Config::LogNetwork)
    LOG_DEBUG << Name << ": sending " << byte_count << " bytes";

  enet_peer_send(peer, 0, packet);
}

void NetworkPeer::HandleAddBullets(const uint8_t* bytes, size_t byte_count)
{
  World& world = World::Get();
  
  const Protocol::Packets::Update<Protocol::BulletDescriptor>* packet = 
    reinterpret_cast<const Protocol::Packets::Update<Protocol::BulletDescriptor>*>(bytes);
  
  std::scoped_lock<std::mutex> lock(EventQueueMutex);
  for (size_t i = 0; i < packet->Count; i++)
  {
    Bullet bullet = packet->Data[i].ToBullet(world.CurrentTime);
    world.GetManager<BulletManager>()->UpdateNextBulletID(bullet.ID);

    EventQueue +=
      std::make_shared<EventsHistory::EventData<EventsHistory::Add<Bullet>>>(
        bullet.Time, bullet, true);
  }
}

void NetworkPeer::HandleUpdateBullets(const uint8_t* bytes, size_t byte_count)
{
}

void NetworkPeer::HandleAddWalls(const uint8_t* bytes, size_t byte_count)
{
  World& world = World::Get();
  
  const Protocol::Packets::Update<Wall>* packet = reinterpret_cast<const Protocol::Packets::Update<Wall>*>(bytes);
  
  std::scoped_lock<std::mutex> lock(EventQueueMutex);
  for (size_t i = 0; i < packet->Count; i++)
  {
    Wall& wall = packet->Data[i];

    world.GetManager<BulletManager>()->UpdateNextWallID(wall.ID);

    double wall_time = wall.Time.GetTime(world.CurrentTime);

    EventQueue.insert(
      std::make_shared<EventsHistory::EventData<EventsHistory::Add<Wall>>>(
        wall_time, wall, true));
  }
}
