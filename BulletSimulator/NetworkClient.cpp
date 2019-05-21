#include "Common.h"

#include "NetworkClient.h"

#include "Logger.h"
#include "Protocol.h"
#include "BulletManager.h"
#include "ConsoleManager.h"

#include <enet/enet.h>


NetworkClient::NetworkClient(std::mutex& EventQueueMutex,
  Set<std::shared_ptr<EventsHistory::Event>, EventsHistory::EventsCompare<>>& EventQueue) :
  NetworkPeer("network client", EventQueueMutex, EventQueue)
{
  if (!(ENet.Host = enet_host_create(
    nullptr /* create a client host */,
    1       /* only allow 1 outgoing connection */,
    2       /* allow up 2 channels to be used, 0 and 1 */,
    0       /* assume any amount of incoming bandwidth */,
    0       /* assume any amount of outgoing bandwidth */)))
  {
    LOG_ERROR << Name << ": an error occurred while trying to create an ENet client host.";
  }

  ConnectHandlers += [this](VoidPointer peer) 
  {
    Sync();
  };

  DisconnectHandlers += [this](VoidPointer peer)
  {
    LOG << Name << ": disconnected from server";
    Done = true;
  };

  PacketHandlers[Protocol::PacketType::WORLD_SYNC] += [this](VoidPointer peer, const uint8_t* data, size_t byte_count)
  {
    if (Config::LogNetwork)
      LOG_DEBUG << Name << ": WORLD_SYNC packet received";

    const Protocol::Packets::WorldSync* packet = reinterpret_cast<const Protocol::Packets::WorldSync*>(data);

    World& world = World::Get();
    
    {
      std::scoped_lock<std::mutex> main_loop_lock(world.MainLoopMutex);

      world.Reset();

      std::scoped_lock<std::mutex> lock(this->EventQueueMutex);
      for (size_t i = 0; i < packet->Header.WallCount; ++i)
      {
        world.GetManager<BulletManager>()->UpdateNextWallID(packet->Walls[i].ID);
        this->EventQueue.insert(
          std::make_shared<EventsHistory::EventData<EventsHistory::Add<Wall>>>(
            packet->Walls[i].Time.GetTime(packet->Time), packet->Walls[i], true));
      }

      for (size_t i = 0; i < packet->Header.BulletCount; ++i)
      {
        world.GetManager<BulletManager>()->UpdateNextBulletID(packet->Bullets[i].ID);
        this->EventQueue.insert(
          std::make_shared<EventsHistory::EventData<EventsHistory::Add<Bullet>>>(
            packet->Bullets[i].Time.GetTime(packet->Time), packet->Bullets[i].ToBullet(packet->Time), true));
      }

      world.CurrentTime = packet->Time;
    }
  };

  PacketHandlers[Protocol::PacketType::ADD] += [this](VoidPointer peer, const uint8_t* data, size_t byte_count)
  {
    if (Config::LogNetwork)
      LOG_DEBUG << Name << ": received ADD packet";
        
    switch (data[1])
    {
    case Protocol::EntityType::BULLET:
    {
      HandleAddBullets(data, byte_count);
    }
    break;
    case Protocol::EntityType::WALL:
    {
      HandleAddWalls(data, byte_count);
    }
    break;
    }
  };
}

void NetworkClient::Connect(const std::string& address_str, int port)
{
  Done = false;

  ENetAddress address;
  
  enet_address_set_host(&address, address_str.c_str());
  address.port = port;
  
  if ((ENet.Peer = enet_host_connect(ENet.Host, &address, 2, 0)) == nullptr)
  {
    LOG_ERROR << Name << ": no available peers for initiating an ENet connection.";
    return;
  }

  LOG << Name << ": opening connection to " << address_str << ":" << port;

  ListenThread = std::make_shared<std::thread>([this]{ Listen(TimeoutMs); });
}

void NetworkClient::Sync()
{
  if (!ENet.Host || !ENet.Peer)
  {
    LOG_WARNING << Name << ": can't sync, no connection";
    return;
  }

  Protocol::Packets::Sync data;

  Send(ENet.Peer, &data, sizeof(data));
}

void NetworkClient::Fire(const Bullet& bullet)
{
  size_t byte_count;
  std::shared_ptr<Protocol::Packets::Update<Protocol::BulletDescriptor>> update = 
    Protocol::Packets::Update<Protocol::BulletDescriptor>::Make(Protocol::PacketType::ADD, 1, byte_count);

  update->Data[0] = { bullet, bullet.Time };

  Send(ENet.Peer, update.get(), byte_count);
}

void NetworkClient::AddWall(const Wall& wall)
{
  size_t byte_count;
  std::shared_ptr<Protocol::Packets::Update<Wall>> update =
    Protocol::Packets::Update<Wall>::Make(Protocol::PacketType::ADD, 1, byte_count);

  update->Data[0] = wall;

  Send(ENet.Peer, update.get(), byte_count);
}
