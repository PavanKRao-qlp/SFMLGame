#pragma once
#include <SFML/Window.hpp>
#include "Input/Input.h"

UMBRA::KeyBoard::Keycode ConvertSFMLKeyCode(sf::Keyboard::Key _key) {
   //copied so no real conversion just for future
    return (UMBRA::KeyBoard::Keycode)_key;
}
UMBRA::Mouse::MouseButton ConvertSFMLMouseCode(sf::Mouse::Button _key) {
   //copied so no real conversion just for future
    return (UMBRA::Mouse::MouseButton)_key;
}