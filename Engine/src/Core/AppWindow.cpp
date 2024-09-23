#pragma once
#include "Core/AppWindow.h"
#include "Diag/Logger.h"
#include "Input/Input.h"
#include "sfmlHelper.h"

AppWindow::AppWindow()
{
}

AppWindow::~AppWindow()
{
    if (mWindow)
        delete mWindow;
}

bool AppWindow::CreateWindow()
{
    Logger::Log(LogType::Verbose, "Creating Window");
    mWindow = new sf::RenderWindow(sf::VideoMode(800, 800), "My window");
    //mWindow->setFramerateLimit(60);
    if (mWindow == nullptr)
    {
        Logger::Log(LogType::FatalError, "Creating Window Failed");
        return false;
    }
    bWindowClosed = false;
    return true;
};

void AppWindow::Update()
{ // run the program as long as the window is open
    if (mWindow->isOpen())
    {
        sf::Event sfEvent;
        while (mWindow->pollEvent(sfEvent))
        {
            if (sfEvent.type == sf::Event::Closed)
            {
                bWindowClosed = true;
                UMBRA::EventBus::FireEvent<AppClosedEvent>(new AppClosedEvent());
            }
            if (sfEvent.type == sf::Event::KeyPressed)
            {
                UMBRA::KeyBoard::Keycode keyPressed = ConvertSFMLKeyCode(sfEvent.key.code);
                UMBRA::KeyPressedEvent *event = new UMBRA::KeyPressedEvent(keyPressed);
                UMBRA::EventBus::FireEvent<UMBRA::KeyPressedEvent>(event);
            }
            if (sfEvent.type == sf::Event::KeyReleased)
            {
                UMBRA::KeyBoard::Keycode keyPressed = ConvertSFMLKeyCode(sfEvent.key.code);
                UMBRA::KeyReleasedEvent *event = new UMBRA::KeyReleasedEvent(keyPressed);
                UMBRA::EventBus::FireEvent<UMBRA::KeyReleasedEvent>(event);
            }
            if (sfEvent.type == sf::Event::MouseButtonPressed)
            {
                UMBRA::Mouse::MouseButton buttonPressed = ConvertSFMLMouseCode(sfEvent.mouseButton.button);
                UMBRA::MouseButtonPressedEvent *event = new UMBRA::MouseButtonPressedEvent(buttonPressed, sfEvent.mouseButton.x, sfEvent.mouseButton.y);
                UMBRA::EventBus::FireEvent<UMBRA::MouseButtonPressedEvent>(event);
            }
            if (sfEvent.type == sf::Event::MouseButtonReleased)
            {
                UMBRA::Mouse::MouseButton buttonPressed = ConvertSFMLMouseCode(sfEvent.mouseButton.button);
                UMBRA::MouseButtonReleasedEvent *event = new UMBRA::MouseButtonReleasedEvent(buttonPressed, sfEvent.mouseButton.x, sfEvent.mouseButton.y);
                UMBRA::EventBus::FireEvent<UMBRA::MouseButtonReleasedEvent>(event);
            }
            if (sfEvent.type == sf::Event::MouseMoved)
            {
                UMBRA::MouseMovedEvent *event = new UMBRA::MouseMovedEvent(sfEvent.mouseMove.x, sfEvent.mouseMove.y);
                UMBRA::EventBus::FireEvent<UMBRA::MouseMovedEvent>(event);
            }
            if (sfEvent.type == sf::Event::JoystickButtonPressed)
            {
                // UMBRA::KeyBoard::Keycode keyPressed = ConvertSFMLKeyCode(event.joystickButton.button);
            }
        }
    }
}

void AppWindow::RefreshDisplay()
{
    mWindow->display();
}

void AppWindow::ClearDisplay()
{
    mWindow->clear(sf::Color::Black);
}

void AppWindow::CloseWindow()
{
    Logger::Log(LogType::Verbose, "Closing Window");
    if (mWindow->isOpen())
        mWindow->close();
}

sf::RenderWindow *AppWindow::GetRenderWindowHandle()
{
    return mWindow;
}