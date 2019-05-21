#pragma once

#include "Set.h"
#include "Map.h"
#include "List.h"
#include "Types.h"
#include "EventsHistory.h"
#include "VoidPointer.h"

#include <mutex>
#include <memory>
#include <thread>


class NetworkPeer
{
public:

  std::string Name = "network";
  
  NetworkPeer(
    const std::string& name, 
    std::mutex& EventQueueMutex,
    Set<std::shared_ptr<EventsHistory::Event>, EventsHistory::EventsCompare<>>& EventQueue):
    Name(name),
    EventQueueMutex(EventQueueMutex),
    EventQueue(EventQueue)
  {}

  virtual ~NetworkPeer();

  void Listen(uint32_t timeout_ms);

  virtual void Fire(const struct Bullet& bullet);

  virtual void AddWall(const struct Wall& wall);

  static inline std::string AddressToString(uint32_t ipAddress);

protected:

  void Send(VoidPointer peer, const void* data, size_t byte_count);

  std::shared_ptr<std::thread> ListenThread = nullptr;

  bool Done = false;

  struct
  {
    VoidPointer Host;
    VoidPointer Peer;
  } ENet;

  typedef std::function<void(VoidPointer event)> ConnectHandler;
  List<ConnectHandler> ConnectHandlers;

  typedef std::function<void(VoidPointer event)> DisconnectHandler;
  List<DisconnectHandler> DisconnectHandlers;

  typedef std::function<void(VoidPointer peer, const uint8_t* data, size_t byte_count)> PacketHandler;
  Map<uint8_t, List<PacketHandler>> PacketHandlers;

  void HandleAddBullets(const uint8_t* bytes, size_t byte_count);
  void HandleUpdateBullets(const uint8_t* bytes, size_t byte_count);
  void HandleAddWalls(const uint8_t* bytes, size_t byte_count);
  
  std::mutex& EventQueueMutex;
  Set<std::shared_ptr<EventsHistory::Event>, EventsHistory::EventsCompare<>>& EventQueue;

};
