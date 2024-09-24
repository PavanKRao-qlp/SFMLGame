#pragma once
#include "Core/AppWindow.h"

#include "Diag/Logger.h"
#include "Input/Input.h"
#include "sfmlHelper.h"

AppWindow::AppWindow() {}

AppWindow::~AppWindow() {
    if (mWindow) {
        delete mWindow;
    }
}

bool AppWindow::CreateWindow() {
    Logger::Log(LogType::Verbose, "Creating Window");
    mWindow = new sf::RenderWindow(sf::VideoMode(800, 800), "My window");
    // mWindow->setFramerateLimit(60);
    if (mWindow == nullptr) {
        Logger::Log(LogType::FatalError, "Creating Window Failed");
        return false;
    }
    bWindowClosed = false;
    return true;
}

void AppWindow::Update() { // run the program as long as the window is open
    if (mWindow->isOpen()) {
        sf::Event sfEvent;
        while (mWindow->pollEvent(sfEvent)) {
            if (sfEvent.type == sf::Event::Closed) {
                bWindowClosed = true;
                Umbra::EventBus::FireEvent<AppClosedEvent>(new AppClosedEvent());
            }
            if (sfEvent.type == sf::Event::KeyPressed) {
                Umbra::KeyBoard::Keycode keyPressed = ConvertSFMLKeyCode(sfEvent.key.code);
                Umbra::KeyPressedEvent* event       = new Umbra::KeyPressedEvent(keyPressed);
                Umbra::EventBus::FireEvent<Umbra::KeyPressedEvent>(event);
            }
            if (sfEvent.type == sf::Event::KeyReleased) {
                Umbra::KeyBoard::Keycode keyPressed = ConvertSFMLKeyCode(sfEvent.key.code);
                Umbra::KeyReleasedEvent* event      = new Umbra::KeyReleasedEvent(keyPressed);
                Umbra::EventBus::FireEvent<Umbra::KeyReleasedEvent>(event);
            }
            if (sfEvent.type == sf::Event::MouseButtonPressed) {
                Umbra::Mouse::MouseButton buttonPressed = ConvertSFMLMouseCode(sfEvent.mouseButton.button);
                Umbra::MouseButtonPressedEvent* event =
                    new Umbra::MouseButtonPressedEvent(buttonPressed, sfEvent.mouseButton.x, sfEvent.mouseButton.y);
                Umbra::EventBus::FireEvent<Umbra::MouseButtonPressedEvent>(event);
            }
            if (sfEvent.type == sf::Event::MouseButtonReleased) {
                Umbra::Mouse::MouseButton buttonPressed = ConvertSFMLMouseCode(sfEvent.mouseButton.button);
                Umbra::MouseButtonReleasedEvent* event =
                    new Umbra::MouseButtonReleasedEvent(buttonPressed, sfEvent.mouseButton.x, sfEvent.mouseButton.y);
                Umbra::EventBus::FireEvent<Umbra::MouseButtonReleasedEvent>(event);
            }
            if (sfEvent.type == sf::Event::MouseMoved) {
                Umbra::MouseMovedEvent* event = new Umbra::MouseMovedEvent(sfEvent.mouseMove.x, sfEvent.mouseMove.y);
                Umbra::EventBus::FireEvent<Umbra::MouseMovedEvent>(event);
            }
            if (sfEvent.type == sf::Event::JoystickButtonPressed) {
                // Umbra::KeyBoard::Keycode keyPressed = ConvertSFMLKeyCode(event.joystickButton.button);
            }
        }
    }
}

void AppWindow::RefreshDisplay() {
    mWindow->display();
}

void AppWindow::ClearDisplay() {
    mWindow->clear(sf::Color::Black);
}

void AppWindow::CloseWindow() {
    Logger::Log(LogType::Verbose, "Closing Window");
    if (mWindow->isOpen()) {
        mWindow->close();
    }
}

sf::RenderWindow* AppWindow::GetRenderWindowHandle() {
    return mWindow;
}
