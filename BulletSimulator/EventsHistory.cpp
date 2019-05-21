#include "Common.h"

#include "EventsHistory.h"

#include "Math.h"
#include "World.h"
#include "Bullet.h"
#include "Config.h"
#include "Logger.h"
#include "StringUtils.h"
#include "BulletManager.h"
#include "Vector2Stream.h"

#include <limits>
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <execution>


std::ostream& operator<<(std::ostream& stream, const EventsHistory::Event& event)
{
  stream 
    << "Event[ID=" << event.ID 
    << ",Time=" << event.Time
    << "]";
  return stream;
}

void EventsHistory::ScheduleEvent(std::shared_ptr<Event> event)
{
  EventsQueue += event;
}

uint64_t EventsHistory::GetTotalSize() const
{
  return EventsHistory::Event::GetTotalEventsSize();
}

void EventsHistory::Cleanup()
{
  double event_min_time = World::Time() - Config::HistoryMaxAge;

  while (EventsLog.PopLastIf([event_min_time](const std::shared_ptr<EventsHistory::Event>& event)
  {
    return event->Time < event_min_time;
  }));
}

bool EventsHistory::ApplyEvent(std::shared_ptr<Event> event)
{
  if (!event) return false;

  if ([&]() 
  {
    if (ApplyEventCastAndCall<Add<Bullet>>(event)) return true;
    if (ApplyEventCastAndCall<Remove<Bullet>>(event)) return true;
    if (ApplyEventCastAndCall<Update<Bullet>>(event)) return true;

    if (ApplyEventCastAndCall<Add<Wall>>(event)) return true;
    if (ApplyEventCastAndCall<Remove<Wall>>(event)) return true;
    if (ApplyEventCastAndCall<Update<Wall>>(event)) return true;

    if (ApplyEventCastAndCall<Collision>(event)) return true;

    return false;
  }())
  {
    World::Time() = event->Time;
    EventsLog += event;
    return true;
  }

  return false;
}

Bullet& UpdateCollision(Bullet& bullet, double time)
{
  World& world = World::Get();

  world.GetManager<BulletManager>()->UpdateBulletCollision(bullet, time);

  return bullet;
}


bool EventsHistory::ApplyEvent(std::shared_ptr<EventData<Add<Bullet>>> event)
{
  ScheduleCollisionEvent(UpdateCollision(World::Get().Bullets.Add(event->Data.Value), event->Data.Value.Time));
  return true;
}

bool EventsHistory::ApplyEvent(std::shared_ptr<EventData<Remove<Bullet>>> event)
{
  World::Get().Bullets.Remove(event->Data.Value.ID);
  return true;
}

bool EventsHistory::ApplyEvent(std::shared_ptr<EventData<Update<Bullet>>> event)
{
  ScheduleCollisionEvent(World::Get().Bullets.Add(event->Data.New));
  return true;
}

bool EventsHistory::ApplyEvent(std::shared_ptr<EventData<Add<Wall>>> event)
{
  World& world = World::Get();
  for (Bullet* bullet : world.GetManager<BulletManager>()->WallAdded(world.Walls.Add(event->Data.Value)))
    ScheduleCollisionEvent(*bullet);
  return true;
}

bool EventsHistory::ApplyEvent(std::shared_ptr<EventData<Remove<Wall>>> event)
{
  World::Get().Walls.Remove(event->Data.Value.ID);
  return true;
}

bool EventsHistory::ApplyEvent(std::shared_ptr<EventData<Update<Wall>>> event)
{
  World& world = World::Get();

  Wall* wall = world.Walls.Get(event->Data.Old.ID);

  for (auto& bullet : world.GetManager<BulletManager>()->WallAdded(wall
    ? (*wall = event->Data.New)
    : world.Walls.Add(event->Data.New)))
  {
    ScheduleCollisionEvent(*bullet);
  }

  return true;
}

bool EventsHistory::ApplyEvent(std::shared_ptr<EventData<Collision>> event)
{
  World& world = World::Get();

  Bullet* bullet = world.Bullets.Get(event->Data.BulletID);

  if (!bullet) return false;

  if (!bullet->Collision.Hits) return false;

  if (bullet->Collision.Time != event->Time) return false;

  if (!bullet->Collision.WallIDs.size()) return false;

  if (event->Data.WallIDs != bullet->Collision.WallIDs) return false;

  Set<Wall::id_t> hit_walls;
  double bullet_time = bullet->Collision.Time;
  {
    EventsHistory::ScopedUpdate<Bullet> update(world.History, bullet_time, *bullet);
    hit_walls = bullet->ApplyCollision();
    world.GetManager<BulletManager>()->UpdateBulletCollision(*bullet, bullet_time);
  }

  if (Config::DestroyWallsOnCollision)
  {
    for (Wall::id_t wall_id : hit_walls)
    {
      Wall* wall = world.Walls.Get(wall_id);
      if (wall) ScheduleEvent<Remove<Wall>>(event->Time, *wall);
    }
  }

  return false;
}

void EventsHistory::Rewind(double time)
{
  World& world = World::Get();

  while (EventsLog.PopFirstIf([this, time, &world](const std::shared_ptr<EventsHistory::Event>& event)
  {
    if (event->Time < time) return false;

    World::Time() = event->Time;

    this->RevertEvent(event);

    return true;
  }));

  World::Time() = time;
}

void EventsHistory::ProcessEventsQueue(double time)
{
  while (EventsQueue.size() && ProcessEventsQueueSingle(time));
}

std::shared_ptr<EventsHistory::Event> EventsHistory::GetNextEvent(double max_time, bool remove)
{
  if (!EventsQueue.size()) 
    return std::shared_ptr<EventsHistory::Event>();

  auto iter = EventsQueue.begin();

  std::shared_ptr<EventsHistory::Event> event = *(iter);

  if (event->Time > max_time)
    return std::shared_ptr<EventsHistory::Event>();

  if (remove) EventsQueue.erase(iter);

  return event;
}

bool EventsHistory::ProcessEventsQueueSingle(double time)
{
  auto event = PopNextEvent(time);

  if (!event) return false;

  if (event->Time < World::Time())
  {
    if (event->Persistant) EventsQueue.Add(event);
    Rewind(event->Time);
    return true;
  }
  
  World::Time() = event->Time;
  
  ApplyEvent(event);

  return true;
}

void EventsHistory::RevertEvent(std::shared_ptr<Event> event)
{
  if ([this, event]() 
  {
    if (RevertEventCastAndCall<EventData<Add<Bullet>>>(event)) return true;
    if (RevertEventCastAndCall<EventData<Remove<Bullet>>>(event)) return true;
    if (RevertEventCastAndCall<EventData<Update<Bullet>>>(event)) return true;

    if (RevertEventCastAndCall<EventData<Add<Wall>>>(event)) return true;
    if (RevertEventCastAndCall<EventData<Remove<Wall>>>(event)) return true;
    if (RevertEventCastAndCall<EventData<Update<Wall>>>(event)) return true;

    if (RevertEventCastAndCall<EventData<Collision>>(event)) return true;

    return false;
  }())
  {
    World::Time() = event->Time;

    if (event->Persistant)
    {
      EventsQueue.Add(event);
    }
  }
}

void EventsHistory::RevertEvent(std::shared_ptr<EventData<Add<Bullet>>> event)
{
  if (!World::Get().Bullets.Remove(event->Data.Value.ID))
  {
    LOG_WARNING << "revert event: no bullet found for id " << event->Data.Value.ID;
  }
}

void EventsHistory::RevertEvent(std::shared_ptr<EventData<Remove<Bullet>>> event)
{
  ScheduleCollisionEvent(UpdateCollision(World::Get().Bullets.Add(event->Data.Value), event->Time));
}

void EventsHistory::RevertEvent(std::shared_ptr<EventData<Update<Bullet>>> event)
{
  World& world = World::Get();
  
  ScheduleCollisionEvent(world.Bullets.Add(event->Data.Old));
}

void EventsHistory::RevertEvent(std::shared_ptr<EventData<Add<Wall>>> event)
{
  if (!World::Get().Walls.Remove(event->Data.Value.ID))
  {
    LOG_WARNING << "revert event: wall does not exit: " << event->Data.Value.ID;
  }
}

void EventsHistory::RevertEvent(std::shared_ptr<EventData<Remove<Wall>>> event)
{
  World& world = World::Get();
  for (auto& bullet : world.GetManager<BulletManager>()->WallAdded(world.Walls.Add(event->Data.Value)))
    ScheduleCollisionEvent(*bullet);
}

void EventsHistory::RevertEvent(std::shared_ptr<EventData<Update<Wall>>> event)
{
  World& world = World::Get();

  Wall* wall = world.Walls.Get(event->Data.New.ID);

  for(auto& bullet: world.GetManager<BulletManager>()->WallAdded(wall
    ? (*wall = event->Data.Old)
    : world.Walls.Add(event->Data.Old)))
  {
    ScheduleCollisionEvent(*bullet);
  }
}

void EventsHistory::RevertEvent(std::shared_ptr<EventData<Collision>> event)
{
  // this never happends
}

void EventsHistory::ScheduleCollisionEvent(const Bullet& bullet)
{
  if (!bullet.Collision.Hits) return;

  if (isinf(bullet.Collision.Time)) return;
  
  World& world = World::Get();
  
  double time = bullet.Collision.Time;

  if (time < world.CurrentTime) return;
  
  ScheduleEvent<EventsHistory::Collision>(time, { bullet.ID, bullet.Collision.WallIDs });
}

void EventsHistory::Clear()
{
  EventsLog.clear();
  EventsQueue.clear();
}

double EventsHistory::GetLastCollisionTime()
{
  if (!EventsLog.size()) return std::numeric_limits<double>::max();

  for (auto event : EventsLog)
  {
    auto collision_event = std::dynamic_pointer_cast<EventsHistory::EventData<Collision>>(event);
    if (collision_event)
    {
      return collision_event->Time;
    }
  }

  return std::numeric_limits<double>::max();
}
