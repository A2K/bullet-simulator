#include "Common.h"

#include "NetworkServer.h"

#include "World.h"
#include "Logger.h"
#include "Protocol.h"
#include "BulletManager.h"
#include "ConsoleManager.h"

#include <enet/enet.h>


NetworkServer::NetworkServer(std::mutex& EventQueueMutex,
  Set<std::shared_ptr<EventsHistory::Event>, EventsHistory::EventsCompare<>>& EventQueue) :
  NetworkPeer("network server", EventQueueMutex, EventQueue)
{
  ConnectHandlers += [this](VoidPointer event) 
  {
    ConnectedPeers.insert(event.Get<ENetEvent>()->peer);
  };

  DisconnectHandlers += [this](VoidPointer event)
  {
    auto iter = ConnectedPeers.find(event.Get<ENetEvent>()->peer);
    if (iter != ConnectedPeers.end())
    {
      ConnectedPeers.erase(iter);
    }
  };

  PacketHandlers[Protocol::PacketType::SYNC] += [this](VoidPointer peer, const uint8_t* sync_packet_bytes, size_t sync_packet_length)
  {
    const char* peer_name = reinterpret_cast<char*>(peer.Get<ENetPeer>()->data);

    if (Config::LogNetwork)
      LOG << Name << ": SYNC packet received from " << peer_name;

    World& world = World::Get();
    std::scoped_lock<std::mutex> lock(world.MainLoopMutex);

    const double time = world.CurrentTime;

    const size_t wall_count = world.Walls.size();
    const size_t bullet_count = world.Bullets.size();
    
    size_t byte_count;
    std::shared_ptr<Protocol::Packets::WorldSync> data = 
      Protocol::Packets::WorldSync::Make(wall_count, bullet_count, byte_count);

    world.Walls.Convert(&data->Walls[0]);

    world.Bullets.ForEach([data, time](const Bullet& bullet, const size_t i)
    {
      data->Bullets[i] = { bullet, time };
    });

    data->Time = world.CurrentTime;

    Send(peer, data.get(), byte_count);
  };

  PacketHandlers[Protocol::PacketType::ADD] += [this](VoidPointer peer, const uint8_t* data, size_t byte_count)
  {
    if (Config::LogNetwork)
      LOG_DEBUG << Name << ": received ADD packet";

    uint32_t count = *reinterpret_cast<const uint32_t*>(data + 2);
    
    switch (data[1])
    {
    case Protocol::EntityType::BULLET:
    {
      World& world = World::Get();

      double min_time = std::numeric_limits<double>::infinity();

      const Protocol::Packets::Update<Protocol::BulletDescriptor>* packet = 
        reinterpret_cast<const Protocol::Packets::Update<Protocol::BulletDescriptor>*>(data);

      Array<Protocol::BulletDescriptor> added_bullets(packet->Count);
      BulletManager* bullet_manager = world.GetManager<BulletManager>();

      std::scoped_lock<std::mutex> lock(this->EventQueueMutex);
      size_t index = 0;
      packet->ForEach([this, &added_bullets, &index, &bullet_manager, &world](const Protocol::BulletDescriptor& bullet, size_t index)
      {
        Bullet copy = bullet.ToBullet(world.CurrentTime);
        copy.ID = bullet_manager->GetNextBulletID();

        added_bullets[index] = bullet;

        this->EventQueue.insert(
          std::make_shared<EventsHistory::EventData<EventsHistory::Add<Bullet>>>(
            bullet.Time, copy, true));
      });

      size_t update_size;
      auto update = Protocol::Packets::Update<Protocol::BulletDescriptor>::Make(Protocol::PacketType::ADD, added_bullets.size(), update_size);
      index = 0;
      for(const Protocol::BulletDescriptor& bullet: added_bullets)
      { 
        update->Data[index++] = bullet;
      }

      ENetPacket* update_packet = enet_packet_create(update.get(), byte_count, ENET_PACKET_FLAG_RELIABLE);
      
      if (Config::LogNetwork)
        LOG_DEBUG << Name << ": broadcasting " << update_size << " bytes ADD[Bullet] packet";

      enet_host_broadcast(ENet.Host, 0, update_packet);
    }
    break;
    case Protocol::EntityType::WALL:
    {
      World& world = World::Get();

      const Protocol::Packets::Update<Wall>* packet = reinterpret_cast<const Protocol::Packets::Update<Wall>*>(data);

      Array<Wall> added_walls(packet->Count);
      std::scoped_lock<std::mutex> lock(this->EventQueueMutex);
      for (size_t i = 0; i < packet->Count; i++)
      {
        Wall wall = packet->Data[i];
        wall.ID = world.GetManager<BulletManager>()->GetNextWallID();

        added_walls[i] = wall;

        double wall_time = wall.Time.GetTime(world.CurrentTime);
        this->EventQueue.insert(
          std::make_shared<EventsHistory::EventData<EventsHistory::Add<Wall>>>(
            wall_time, wall, true));
      }

      size_t update_size;
      auto update = Protocol::Packets::Update<Wall>::Make(Protocol::PacketType::ADD, added_walls.size(), update_size);
      size_t index = 0;
      for (const Wall& wall: added_walls)
      {
        update->Data[index++] = wall;
      }

      ENetPacket* update_packet = enet_packet_create(update.get(), byte_count, ENET_PACKET_FLAG_RELIABLE);

      if (Config::LogNetwork)
        LOG_DEBUG << Name << ": broadcasting " << update_size << " bytes ADD[Bullet] packet";

      enet_host_broadcast(ENet.Host, 0, update_packet);
    }
    break;
    }
  };
}

NetworkServer::~NetworkServer()
{
  for(auto& Peer: ConnectedPeers)
  {
    enet_peer_disconnect(Peer, 0);
  }
}

void NetworkServer::Bind(int port)
{
  Done = false;

  ENetAddress address;

  address.host = ENET_HOST_ANY;

  address.port = port;

  ENet.Host = enet_host_create(&address, MaxClients, Channels,
    0 /* incoming bandwidth (0 = unlimited)*/,
    0 /* outgoing bandwidth (0 = unlimited)*/);

  if (ENet.Host == nullptr)
  {
    Done = true;
    LOG_ERROR << Name << ": an error occurred while trying to create an ENet server host";
    return;
  }

  Listening = true;

  LOG_SUCCESS << Name << ": bound to port " << port;

  ListenThread = std::make_shared<std::thread>([this] { Listen(TimeoutMs); });
}

void NetworkServer::Fire(const Bullet& bullet)
{
  size_t byte_count;
  std::shared_ptr<Protocol::Packets::Update<Protocol::BulletDescriptor>> update =
    Protocol::Packets::Update<Protocol::BulletDescriptor>::Make(Protocol::PacketType::ADD, 1, byte_count);

  update->Data[0] = { bullet, bullet.Time };

  ENetPacket* packet = enet_packet_create(update.get(), byte_count, ENET_PACKET_FLAG_RELIABLE);

  if (Config::LogNetwork)
    LOG_DEBUG << Name << ": broadcasting " << byte_count << " bytes ADD[Bullet] packet";

  enet_host_broadcast(ENet.Host, 0, packet);
}

void NetworkServer::AddWall(const Wall& wall)
{
  size_t byte_count;
  std::shared_ptr<Protocol::Packets::Update<Wall>> update =
    Protocol::Packets::Update<Wall>::Make(Protocol::PacketType::ADD, 1, byte_count);

  update->Data[0] = wall;

  ENetPacket* packet = enet_packet_create(update.get(), byte_count, ENET_PACKET_FLAG_RELIABLE);

  if (Config::LogNetwork)
    LOG_DEBUG << Name << ": broadcasting " << byte_count << " bytes ADD[Wall] packet";

  enet_host_broadcast(ENet.Host, 0, packet);
}
