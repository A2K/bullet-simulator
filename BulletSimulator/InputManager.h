#pragma once

#include "Map.h"
#include "List.h"
#include "Types.h"
#include "Manager.h"
#include "Vector2.h"

#include <chrono>
#include <functional>


class InputManager : public Manager
{

public:


  enum InputActionId
  {
    INVALID_ID = 0,
    AXIS,
    FIRE_ON,
    FIRE_OFF,
    WALL_START,
    WALL_MOVE,
    WALL_END,
    QUIT,
    FULLSCREEN,
    TIME_REVERSE_ON,
    TIME_REVERSE_OFF,
    CONSOLE,
    DEBUG_OVERLAY,
    DEBUG_HISTORY,
    TIME_SPEED_0X,
    TIME_SPEED_1X,
    TIME_SPEED_3X,
    TIME_SPEED_5X,
    TIME_SPEED_10X,
    TIME_SPEED_25X,
    TIME_SPEED_50X,
    TIME_SPEED_100X,
    TIME_SPEED_500X,
    TIME_SPEED_1000X,
    LASER_TOGGLE,
    RESET_WORLD
  };

  enum AxisId 
  {
    NO_AXIS = 0,
    HORIZONTAL,
    VERTICAL,
    MAX_AXIS_ID
  };

  struct InputAction
  {
    enum InputActionId ActionId = INVALID_ID;
    enum AxisId Axis = NO_AXIS;
    bool InvertAxis = false;

    InputAction(InputActionId ActionId = INVALID_ID, AxisId Axis = NO_AXIS, bool InvertAxis = false) :
      ActionId(ActionId), Axis(Axis), InvertAxis(InvertAxis)
    {}

    bool operator<(const InputAction& other) const
    {
      if (Axis != other.Axis) return Axis < other.Axis;
      if (InvertAxis != other.InvertAxis) 
        return InvertAxis < other.InvertAxis;
      return ActionId < other.ActionId;
    }

    bool operator==(const InputAction& other) const
    {
      return ActionId == other.ActionId 
        && Axis == other.Axis 
        && InvertAxis == other.InvertAxis;
    }

  };

  enum InputDevice
  {
    KEYBOARD,
    MOUSE
  };

  enum ButtonAction
  {
    PRESSED,
    RELEASED,
    HOLD
  };

  struct InputButton
  {
    enum InputDevice Device;
    int Code;

    InputButton(InputDevice Device, int Code):
      Device(Device), Code(Code)
    {}

    bool operator==(const InputButton& other) const
    {
      return Device == other.Device && Code == other.Code;
    }

    bool operator<(const InputButton& other) const
    {
      return (Device == other.Device)
          ? Code < other.Code
          : Device < other.Device;
    }

    friend std::ostream& operator<<(std::ostream& stream, const float2& vector);
  };

  struct InputButtonAction
  {
    InputButton Button;
    enum ButtonAction Action;

    InputButtonAction(const InputButton& button, enum ButtonAction action = PRESSED):
      Button(button), Action(action)
    {}

    bool operator==(const InputButtonAction& other) const
    {
      return Button == other.Button && Action == other.Action;
    }

    bool operator<(const InputButtonAction& other) const
    {
      return (Action == other.Action)
        ? Button < other.Button
        : Action < other.Action;
    }
  };

private:

  Map<InputButton, float> Pressed;

public:

  Map<InputButtonAction, InputAction> InputMap;

  
  typedef std::function<void(float value, double time)> AxisActionDelegate;
  Map<AxisId, AxisActionDelegate> AxisActionDelegates;

  typedef std::function<void(double time)> ActionDelegate;
  Map<InputAction, ActionDelegate> ActionDelegates;

  typedef std::function<void(int button, const float2& location, double time)> MouseActionDelegate;
  Map<InputAction, MouseActionDelegate> MouseActionDelegates;

  typedef std::function<void(float amount, const float2& location, double time)> MouseWheelDelegate;
  List<MouseWheelDelegate> MouseWheelDelegates;

  InputManager() {}

  InputManager(const Map<InputButtonAction, InputAction>& InputMap):
    InputMap(InputMap)
  {}

  void KeyPressed(const InputButton& button, double time);

  void KeyReleased(const InputButton& button, double time);

  bool IsKeyPressed(const InputButton& button, double* time = nullptr);

  void MouseButtonPressed(const InputButton& button, const float2& location, double time);

  void MouseButtonReleased(const InputButton& button, const float2& location, double time);

  void MouseMoved(const float2& location, double time);

  void MouseWheel(const float amount, const float2& location, double time);

  float2& UpdateMouseLocation();

  void Update(double delta_time) override;
    
  float2 MouseLocation;
};
