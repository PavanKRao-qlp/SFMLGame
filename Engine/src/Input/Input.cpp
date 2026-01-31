#include "Input/Input.h"

namespace Umbra {

    Input::Input() {}

    Input::~Input() {
        // Cleanup handled by Singleton base class
    }

    void Input::OnKeyPressed(const KeyPressedEvent& event) {
        if (event.Key >= 0 && event.Key < KeyBoard::Keycode::COUNT) {
            KeyPressedState.set(event.Key, true);
        }
    }

    void Input::OnKeyReleased(const KeyReleasedEvent& event) {
        if (event.Key >= 0 && event.Key < KeyBoard::Keycode::COUNT) {
            KeyPressedState.set(event.Key, false);
        }
    }

    void Input::OnMouseButtonPressed(const MouseButtonPressedEvent& event) {
        if (event.Button >= 0 && event.Button < Mouse::MouseButton::COUNT) {
            MouseButtonPressedState.set(event.Button, true);
        }
    }

    void Input::OnMouseButtonReleased(const MouseButtonReleasedEvent& event) {
        if (event.Button >= 0 && event.Button < Mouse::MouseButton::COUNT) {
            MouseButtonPressedState.set(event.Button, false);
        }
    }

    void Input::OnMouseMoved(const MouseMovedEvent& event) {
        MousePosition.x = event.x;
        MousePosition.y = event.y;
    }

    bool Input::GetMouseButton(Mouse::MouseButton _button) {
        if (_button < 0 || _button >= Mouse::MouseButton::COUNT) {
            return false;
        }
        const Input* instance = GetInstance();
        return instance->MouseButtonPressedState.test(_button);
    }

    bool Input::GetMouseButtonDown(Mouse::MouseButton _button) {
        if (_button < 0 || _button >= Mouse::MouseButton::COUNT) {
            return false;
        }
        const Input* instance = GetInstance();
        return !instance->PrevMouseButtonPressedState.test(_button)
            && instance->MouseButtonPressedState.test(_button);
    }

    bool Input::GetMouseButtonUp(Mouse::MouseButton _button) {
        if (_button < 0 || _button >= Mouse::MouseButton::COUNT) {
            return false;
        }
        const Input* instance = GetInstance();
        return instance->PrevMouseButtonPressedState.test(_button)
            && !instance->MouseButtonPressedState.test(_button);
    }

    Math::Vector2i Input::GetMousePosition() {
        const Input* instance = GetInstance();
        return instance->MousePosition;
    }

    bool Input::GetKey(KeyBoard::Keycode _code) {
        // Check if key is currently held down
        if (_code < 0 || _code >= KeyBoard::Keycode::COUNT) {
            return false;
        }
        const Input* instance = GetInstance();
        return instance->KeyPressedState.test(_code);
    }

    bool Input::GetKeyDown(KeyBoard::Keycode _code) {
        // Check if key was just pressed this frame
        if (_code < 0 || _code >= KeyBoard::Keycode::COUNT) {
            return false;
        }
        const Input* instance = GetInstance();
        return !instance->PrevKeyPressedState.test(_code) && instance->KeyPressedState.test(_code);
    }

    bool Input::GetKeyUp(KeyBoard::Keycode _code) {
        if (_code < 0 || _code >= KeyBoard::Keycode::COUNT) {
            return false;
        }
        const Input* instance = GetInstance();
        return instance->PrevKeyPressedState.test(_code) && !instance->KeyPressedState.test(_code);
    }

    void Input::RefreshImpl() {
        PrevMouseButtonPressedState = MouseButtonPressedState;
        PrevKeyPressedState         = KeyPressedState;
    }
} // namespace  Umbra
