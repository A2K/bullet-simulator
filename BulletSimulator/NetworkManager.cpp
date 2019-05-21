#include "Common.h"

#include "NetworkManager.h"

#include "NetworkClient.h"
#include "NetworkServer.h"

#include "Wall.h"
#include "Bullet.h"
#include "Logger.h"

#include <enet/enet.h>


static bool enet_initialized = false;

NetworkManager::NetworkManager()
{
  if (!enet_initialized)
  {
    if (enet_initialize() == 0)
    {
      LOG << "network manager: ENet " 
        << ENET_VERSION_MAJOR << "." << ENET_VERSION_MINOR << "." << ENET_VERSION_PATCH
        << " initialized";
      enet_initialized = true;
    }
    else
    {
      LOG << "network manager: failed to initialize ENet";
    }
  }
}

NetworkManager::~NetworkManager()
{
  if (Network.Client)
  {
    delete Network.Client;
    Network.Client = nullptr;
  }
  if (Network.Server)
  {
    delete Network.Server;
    Network.Server = nullptr;
  }

  if (enet_initialized)
  {
    enet_deinitialize();
    enet_initialized = false;
  }
}

void NetworkManager::Stop()
{
  if (Network.Client)
  {
    LOG << "network manager: stopping old client";
    delete Network.Client;
    Network.Client = nullptr;
  }
  if (Network.Server)
  {
    LOG << "network manager: stopping old server";
    delete Network.Server;
    Network.Server = nullptr;
  }
}

void NetworkManager::Listen(int port)
{
  Stop();

  LOG << "network manager: starting server at " << port;

  Network.Server = new NetworkServer(EventQueueMutex, EventQueue);
  Network.Server->Bind(port);
}

void NetworkManager::Connect(const std::string& host, int port)
{
  Stop();

  LOG << "network manager: connecting to " << host << ":" << port;

  Network.Client = new NetworkClient(EventQueueMutex, EventQueue);
  Network.Client->Connect(host, port);
}

void NetworkManager::Fire(const Bullet& bullet)
{
  if (this->Network.Client)
  {
    this->Network.Client->Fire(bullet);
  }
  else if (this->Network.Server)
  {
    this->Network.Server->Fire(bullet);
  }
}

void NetworkManager::AddWall(const Wall& wall)
{
  if (this->Network.Client)
  {
    this->Network.Client->AddWall(wall);
  }
  else if (this->Network.Server)
  {
    this->Network.Server->AddWall(wall);
  }
}
