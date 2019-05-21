#pragma once

#include <thread>
#include <memory>
#include <functional>

#include "Types.h"
#include "NetworkPeer.h"
#include "VoidPointer.h"


class NetworkServer: public NetworkPeer
{
public:

  NetworkServer(std::mutex& EventQueueMutex,
    Set<std::shared_ptr<EventsHistory::Event>, EventsHistory::EventsCompare<>>& EventQueue);

  virtual ~NetworkServer();

  void Bind(int port);

  void Fire(const struct Bullet& bullet) override;

  void AddWall(const struct Wall& bullet) override;

  int MaxClients = 32;
  int Channels = 2;

  int TimeoutMs = 100;

  bool Listening = false;

  bool Done = false;

private:
  
  Set<VoidPointer> ConnectedPeers;

};

