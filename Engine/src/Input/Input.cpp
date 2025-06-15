#include "Input/Input.h"

namespace Umbra {

    Input::Input() {}

    Input::~Input() {
        if (GetInstance()) {
        }
    }

    void Input::OnKeyPressed(const KeyPressedEvent& event) {
        KeyPressedState.set(event.Key, true);
    }

    void Input::OnKeyReleased(const KeyReleasedEvent& event) {
        KeyPressedState.set(event.Key, false);
    }

    void Input::OnMouseButtonPressed(const MouseButtonPressedEvent& event) {
        MouseButtonPressedState.set(event.Button, true);
        MousePosition.x = event.x;
        MousePosition.y = event.y;
    }

    void Input::OnMouseButtonReleased(const MouseButtonReleasedEvent& event) {
        MouseButtonPressedState.set(event.Button, false);
        MousePosition.x = event.x;
        MousePosition.y = event.y;
    }

    void Input::OnMouseMoved(const MouseMovedEvent& event) {
        MousePosition.x = event.x;
        MousePosition.y = event.y;
    }

    bool Input::GetMouseButton(Mouse::MouseButton _button) {
        return GetInstance()->MouseButtonPressedState.test(_button);
    }

    bool Input::GetMouseButtonDown(Mouse::MouseButton _button) {
        return !GetInstance()->PrevMouseButtonPressedState.test(_button)
            && GetInstance()->MouseButtonPressedState.test(_button);
    }

    bool Input::GetMouseButtonUp(Mouse::MouseButton _button) {
        return GetInstance()->PrevMouseButtonPressedState.test(_button)
            && !GetInstance()->MouseButtonPressedState.test(_button);
    }

    Math::Vector2i Input::GetMousePosition() {
        return GetInstance()->MousePosition;
    }

    bool Input::GetKey(KeyBoard::Keycode _code) {

        return !GetInstance()->PrevKeyPressedState.test(_code) && GetInstance()->KeyPressedState.test(_code);
    }

    bool Input::GetKeyDown(KeyBoard::Keycode _code) {
        return GetInstance()->KeyPressedState.test(_code);
    }

    bool Input::GetKeyUp(KeyBoard::Keycode _code) {
        return GetInstance()->PrevKeyPressedState.test(_code) && !GetInstance()->KeyPressedState.test(_code);
    }

    void Input::RefreshImpl() {
        PrevMouseButtonPressedState = MouseButtonPressedState;
        PrevKeyPressedState         = KeyPressedState;
    }
} // namespace  Umbra
