#pragma once

#include <string>
#include <memory>

#include "EventsHistory.h"
#include "NetworkPeer.h"


struct Wall;
struct Bullet;

class NetworkClient: public NetworkPeer
{
public:

  NetworkClient(std::mutex& EventQueueMutex,
    Set<std::shared_ptr<EventsHistory::Event>, EventsHistory::EventsCompare<>>& EventQueue);
  
  void Connect(const std::string& host, int port);
  
  void Sync();

  void Fire(const struct Bullet& bullet) override;

  void AddWall(const struct Wall& bullet) override;

  bool Done = false;

  int TimeoutMs = 100;

private:

  bool Connected = false;  

};
