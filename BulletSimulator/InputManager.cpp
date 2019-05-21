#include "Common.h"

#include "InputManager.h"

#include "World.h"
#include "Config.h"
#include "Logger.h"

#include <algorithm>


Map<int, float> Pressed;

void InputManager::KeyPressed(const InputButton& button, double time)
{
  auto iter = Pressed.find(button);
  if (iter == Pressed.end())
  {
    Pressed[button] = time;
    auto actionIter = InputMap.find(InputButtonAction(button, PRESSED));
    if (actionIter != InputMap.end())
    {
      auto action_delegate = ActionDelegates.find(actionIter->second);
      if (action_delegate != ActionDelegates.end())
      {
        action_delegate->second(time);
      }
    }
  }
}

void InputManager::KeyReleased(const InputButton& button, double time)
{
  auto iter = Pressed.find(button);
  if (iter != Pressed.end())
  {
    Pressed.erase(iter);

    auto actionIter = InputMap.find(InputButtonAction(button, RELEASED));
    if (actionIter != InputMap.end())
    {
      auto action_delegate = ActionDelegates.find(actionIter->second);
      if (action_delegate != ActionDelegates.end())
      {
        action_delegate->second(time);
      }
    }
  }
}

void InputManager::MouseButtonPressed(const InputButton& button, const float2& location, double time)
{
  this->MouseLocation = location;
  auto iter = Pressed.find(button);
  if (iter == Pressed.end())
  {
    auto actionIter = InputMap.find(InputButtonAction(button, PRESSED));
    if (actionIter != InputMap.end())
    {
      Pressed[button] = time;
      auto action_delegate = MouseActionDelegates.find(actionIter->second);
      if (action_delegate != MouseActionDelegates.end())
      {
        action_delegate->second(button.Code, location, time);
      }
    }
  }
}

void InputManager::MouseButtonReleased(const InputButton& button, const float2& location, double time)
{
  this->MouseLocation = location;
  auto iter = Pressed.find(button);
  if (iter != Pressed.end())
  {
    Pressed.erase(iter);
    auto actionIter = InputMap.find(InputButtonAction(button, RELEASED));
    if (actionIter != InputMap.end())
    {
      auto action_delegate = MouseActionDelegates.find(actionIter->second);
      if (action_delegate != MouseActionDelegates.end())
      {
        action_delegate->second(button.Code, location, time);
      }
    }
  }
}

void InputManager::MouseMoved(const float2& location, double time)
{
  this->MouseLocation = location;
  auto actionIter = InputMap.find(InputButtonAction(InputButton(MOUSE, -1), HOLD));
  if (actionIter != InputMap.end())
  {
    auto action_delegate = MouseActionDelegates.find(actionIter->second);
    if (action_delegate != MouseActionDelegates.end())
    {
      action_delegate->second(-1, location, time);
    }
  }
}

void InputManager::MouseWheel(const float amount, const float2& location, double time)
{
  for (auto wheel_delegate : MouseWheelDelegates)
  {
    wheel_delegate(amount, location, time);
  }
}

bool InputManager::IsKeyPressed(const InputButton& button, double* time)
{
  auto iter = Pressed.find(button);
  if (iter != Pressed.end())
  {
    if (time!= nullptr)
    {
      *time = iter->second;
    }
    return true;
  }
  return false;
}

float2& InputManager::UpdateMouseLocation()
{
  int x_, y_;
  SDL_GetMouseState(&x_, &y_);
  return MouseLocation = (float2(x_, y_) / Config::RenderScale - World::Get().GetRenderOffset());
}

void InputManager::Update(double delta_time)
{
  UpdateMouseLocation();

  float axis_values[MAX_AXIS_ID];

  for (size_t i = 0; i < MAX_AXIS_ID; ++i)
  {
    axis_values[i] = 0;
  }

  for (auto& pair : InputMap)
  {
    if (pair.second.Axis != NO_AXIS)
    {
      double pressed_time;
      if (IsKeyPressed(pair.first.Button, &pressed_time))
      {
        double pressed_seconds = World::Get().CurrentTime - pressed_time;
        double update_time = std::min(double(delta_time), pressed_seconds);

        axis_values[pair.second.Axis] += (pair.second.InvertAxis ? -1 : 1) * update_time / double(delta_time);
      }
    }
  }

  for (size_t i = 0; i < MAX_AXIS_ID; ++i)
  {
    enum AxisId axis = (AxisId)i;
    auto iter = AxisActionDelegates.find(axis);
    if (iter != AxisActionDelegates.end())
    {
      iter->second(axis_values[i], delta_time);
    }
  }
}

std::ostream& operator<<(std::ostream& stream, const InputManager::InputButton& button)
{
  stream 
    << "InputButton(Device="
    << (button.Device == InputManager::MOUSE ? "Mouse" : "Keyboard")
    << ",Code=" << button.Code << ")";
  return stream;
}
