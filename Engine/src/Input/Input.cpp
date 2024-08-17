#include "Input/Input.h"

namespace D2D
{
    Input::Input()
    {
        EventBus::Subscribe<KeyPressedEvent>(&Input::OnKeyPressed);
        EventBus::Subscribe<KeyReleasedEvent>(&Input::OnKeyReleased);
        EventBus::Subscribe<MouseButtonPressedEvent>(&Input::OnMouseButtonPressed);
        EventBus::Subscribe<MouseButtonReleasedEvent>(&Input::OnMouseButtonReleased);
        EventBus::Subscribe<MouseMovedEvent>(&Input::OnMouseMoved);
    }

    Input::~Input()
    {
    }

    void Input::OnKeyPressed(const KeyPressedEvent &event)
    {
        PrevKeyPressedState.set(event.Key, KeyPressedState.test(event.Key));
        KeyPressedState.set(event.Key, true);
    }

    void Input::OnKeyReleased(const KeyReleasedEvent &event)
    {
        PrevKeyPressedState.set(event.Key, KeyPressedState.test(event.Key));
        KeyPressedState.set(event.Key, false);
    }

     void Input::OnMouseButtonPressed(const MouseButtonPressedEvent &event)
     {
        PrevMouseButtonPressedState.set(event.Button, MouseButtonPressedState.test(event.Button));
        KeyPressedState.set(event.Button, true);
        MouseX = event.x;
        MouseY = event.y;
     }

    void Input::OnMouseButtonReleased(const MouseButtonReleasedEvent &event)
    {
        PrevMouseButtonPressedState.set(event.Button, MouseButtonPressedState.test(event.Button));
        KeyPressedState.set(event.Button, false);
        MouseX = event.x;
        MouseY = event.y;
    }

    void Input::OnMouseMoved(const MouseMovedEvent &event)
    {
        MouseX = event.x;
        MouseY = event.y;
    }

    bool Input::GetMouseButton(Mouse::MouseButton _button)
    {
        return !PrevMouseButtonPressedState.test(_button) && MouseButtonPressedState.test(_button);
    }

    bool Input::GetMouseButtonDown(Mouse::MouseButton _button)
    {
        return !PrevMouseButtonPressedState.test(_button) && MouseButtonPressedState.test(_button);
    }

    bool Input::GetMouseButtonUp(Mouse::MouseButton _button)
    {
        return !PrevMouseButtonPressedState.test(_button) && MouseButtonPressedState.test(_button);
    }

    int16 Input::GetMousePositionX(){
        return MouseX;
    }

    int16 Input::GetMousePositionY(){
        return MouseY;
    }

    bool Input::GetKey(KeyBoard::Keycode _code)
    {
        return !PrevKeyPressedState.test(_code) && KeyPressedState.test(_code);
    }

    bool Input::GetKeyDown(KeyBoard::Keycode _code)
    {
        return KeyPressedState.test(_code);
    }

    bool Input::GetKeyUp(KeyBoard::Keycode _code)
    {
        return PrevKeyPressedState.test(_code) && !KeyPressedState.test(_code);
    }
} // namespace  D2D
