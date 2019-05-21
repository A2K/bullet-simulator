#pragma once

#include "Types.h"
#include "EventsHistory.h"
#include "Manager.h"

#include <mutex>
#include <string>


class NetworkManager: public Manager
{
public:

public:

  NetworkManager();

  ~NetworkManager();

  void Listen(int port);

  void Connect(const std::string& host, int port);
  
  void Stop();
  
  void Fire(const struct Bullet& bullet);

  void AddWall(const struct Wall& wall);

  struct {
    class NetworkClient* Client = nullptr;
    class NetworkServer* Server = nullptr;
  } Network;
  
  std::mutex EventQueueMutex;
  Set<std::shared_ptr<EventsHistory::Event>, EventsHistory::EventsCompare<std::less>> EventQueue;

};

