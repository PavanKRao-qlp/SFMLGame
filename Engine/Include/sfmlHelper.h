#pragma once
#include <SFML/Window.hpp>
#include "Input/Input.h"

D2D::KeyBoard::Keycode ConvertSFMLKeyCode(sf::Keyboard::Key _key) {
   //copied so no real conversion just for future
    return (D2D::KeyBoard::Keycode)_key;
}
D2D::Mouse::MouseButton ConvertSFMLMouseCode(sf::Mouse::Button _key) {
   //copied so no real conversion just for future
    return (D2D::Mouse::MouseButton)_key;
}