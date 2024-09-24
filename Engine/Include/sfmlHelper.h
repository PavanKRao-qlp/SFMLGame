#pragma once
#include "Input/Input.h"

#include <SFML/Window.hpp>

Umbra::KeyBoard::Keycode ConvertSFMLKeyCode(sf::Keyboard::Key _key) {
    // copied so no real conversion just for future
    return (Umbra::KeyBoard::Keycode) _key;
}
Umbra::Mouse::MouseButton ConvertSFMLMouseCode(sf::Mouse::Button _key) {
    // copied so no real conversion just for future
    return (Umbra::Mouse::MouseButton) _key;
}
