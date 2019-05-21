#pragma once

#include "Set.h"
#include "Types.h"

#include <atomic>
#include <memory>


struct Wall;
struct Bullet;

struct EventsHistory
{
  enum EventType
  {
    ET_ADD,
    ET_REMOVE,
    ET_UPDATE,
    ET_COLLISION
  };

  class Event
  {
  public:

    uint64_t ID;
    double Time;

    EventType Type;
    bool Persistant = false;

    size_t Size;

    Event(double time, EventType type, size_t size, bool persistant = false) :
      Time(time), Type(type), Size(size), Persistant(persistant)
    {
      static std::atomic<uint64_t> EventId = 0;
      ID = EventId++;

      GetTotalEventsSizeRef() += Size;
    }

    static uint64_t GetTotalEventsSize()
    {
      return GetTotalEventsSizeRef();
    }

    virtual ~Event()
    {
      GetTotalEventsSizeRef() -= Size;
    }

    virtual bool operator < (const Event& other) const
    {
      return (Time == other.Time)
        ? ID < other.ID
        : Time < other.Time;
    }

  private:

    static std::atomic<uint64_t>& GetTotalEventsSizeRef()
    {
      static std::atomic<uint64_t> total_events_size = 0;
      return total_events_size;
    }
  };

  template<typename T>
  struct Add
  {
    static const EventType TYPE = ET_ADD;
    T Value;
    Add() {}
    Add(const T& value) :
      Value(value) {}
  };

  template<typename T>
  struct Remove
  {
    static const EventType TYPE = ET_REMOVE;
    T Value;
    Remove() {}
    Remove(const T& value) :
      Value(value) {}
  };

  template<typename T>
  struct Update
  {
    static const EventType TYPE = ET_UPDATE;
    T Old;
    T New;
    Update() {}
    Update(const T& old, const T& new_):
      Old(old), New(new_) {}
  };

  struct Collision
  {
    static const EventType TYPE = ET_COLLISION;
    uint32_t BulletID;
    Set<uint32_t> WallIDs;
    Collision(uint32_t bulletID, const Set<uint32_t>& wallIDs):
      BulletID(bulletID), WallIDs(wallIDs)
    {}
  };

  template<typename T>
  class EventData : public virtual Event
  {
  public:

    T Data;

    EventData(double time, const T& Data, bool persistant = false) :
      Event(time, T::TYPE, sizeof(EventData<T>), persistant), Data(Data)
    {}

    virtual ~EventData() {}
  };

  template<typename T, template<class> typename E = Update>
  struct ScopedUpdate
  {
    EventsHistory& History;

    double Time;

    const T& Target;

    T InitialValue;

    ScopedUpdate(EventsHistory& History, double time, const T& target) :
      History(History),
      Time(time),
      Target(target),
      InitialValue(target)
    {
    }

    virtual ~ScopedUpdate()
    {
      History.ScheduleEvent<E<T>>(Time, { InitialValue, Target });
    }
  };

  template<template<class>typename Cmp = std::less>
  struct EventsCompare
  {
    bool operator()(std::shared_ptr<Event> _Left, std::shared_ptr<Event> _Right) const
    {
      static struct {
        Cmp<uint64_t> id;
        Cmp<double> time;
      } cmp;
      if (_Left->Time == _Right->Time)
        return cmp.id(_Left->ID, _Right->ID);
      return cmp.time(_Left->Time, _Right->Time);
    }
  };

public:

  Set<std::shared_ptr<Event>, EventsCompare<std::greater>> EventsLog;

  Set<std::shared_ptr<Event>, EventsCompare<std::less>> EventsQueue;

  const float MaxAge = 60.0; // seconds

public:

  uint64_t GetTotalSize() const;

  void ScheduleEvent(std::shared_ptr<Event> event);

  template<typename T>
  void ScheduleEvent(double time, const T& Data, const bool persistant = false)
  {
    return ScheduleEvent(std::make_shared<EventData<T>>(time, Data, persistant));
  }

  template<typename T>
  void ScheduleEvent(double time, const T* Data, const bool persistant = false)
  {
    return ScheduleEvent(std::make_shared<EventData<T>>(time, *Data, persistant));
  }

  template<typename T>
  size_t GetEventSizeBytes(std::shared_ptr<EventData<T>> Event)
  {
    return sizeof(EventData<T>);
  }

  template<typename T, typename BaseT>
  inline bool ApplyEventCastAndCall(std::shared_ptr<BaseT> event)
  {
    std::shared_ptr<EventData<T>> ptr = std::dynamic_pointer_cast<EventData<T>>(event);
    if (!ptr) return false;    
    return ApplyEvent(ptr);
  }    
  bool ApplyEvent(std::shared_ptr<Event> event);
  bool ApplyEvent(std::shared_ptr<EventData<Add<Bullet>>> event);
  bool ApplyEvent(std::shared_ptr<EventData<Remove<Bullet>>> event);
  bool ApplyEvent(std::shared_ptr<EventData<Update<Bullet>>> event);
  bool ApplyEvent(std::shared_ptr<EventData<Add<Wall>>> event);
  bool ApplyEvent(std::shared_ptr<EventData<Remove<Wall>>> event);
  bool ApplyEvent(std::shared_ptr<EventData<Update<Wall>>> event);
  bool ApplyEvent(std::shared_ptr<EventData<Collision>> event);

  template<typename T, typename BaseT>
  bool RevertEventCastAndCall(std::shared_ptr<BaseT> event)
  {
    std::shared_ptr<T> ptr = std::dynamic_pointer_cast<T>(event);
    if (!ptr) return false;
    RevertEvent(ptr);
    return true;
  }
  void RevertEvent(std::shared_ptr<Event> event);
  void RevertEvent(std::shared_ptr<EventData<Add<Bullet>>> event);
  void RevertEvent(std::shared_ptr<EventData<Remove<Bullet>>> event);
  void RevertEvent(std::shared_ptr<EventData<Update<Bullet>>> event);
  void RevertEvent(std::shared_ptr<EventData<Add<Wall>>> event);
  void RevertEvent(std::shared_ptr<EventData<Remove<Wall>>> event);
  void RevertEvent(std::shared_ptr<EventData<Update<Wall>>> event);
  void RevertEvent(std::shared_ptr<EventData<Collision>> event);
  
  void Rewind(double time);
  bool ProcessEventsQueueSingle(double time);
  void ProcessEventsQueue(double time);
    
  void Cleanup();

  void Clear();

  std::shared_ptr<Event> GetNextEvent(double max_time = std::numeric_limits<double>::infinity(), bool remove = false);

  std::shared_ptr<Event> PopNextEvent(double max_time = std::numeric_limits<double>::infinity())
  {
    return GetNextEvent(max_time, true);
  }
  
  void ScheduleCollisionEvent(const Bullet& bullet);

  double GetLastCollisionTime();
};

extern std::ostream& operator<<(std::ostream& stream, const EventsHistory::Event& event);

inline std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<EventsHistory::Event>& event)
{
  return stream << (*event);
}
