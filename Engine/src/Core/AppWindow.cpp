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
                D2D::EventBus::FireEvent<AppClosedEvent>(new AppClosedEvent());
            }
            if (sfEvent.type == sf::Event::KeyPressed)
            {
                D2D::KeyBoard::Keycode keyPressed = ConvertSFMLKeyCode(sfEvent.key.code);
                D2D::KeyPressedEvent *event = new D2D::KeyPressedEvent(keyPressed);
                D2D::EventBus::FireEvent<D2D::KeyPressedEvent>(event);
            }
            if (sfEvent.type == sf::Event::KeyReleased)
            {
                D2D::KeyBoard::Keycode keyPressed = ConvertSFMLKeyCode(sfEvent.key.code);
                D2D::KeyReleasedEvent *event = new D2D::KeyReleasedEvent(keyPressed);
                D2D::EventBus::FireEvent<D2D::KeyReleasedEvent>(event);
            }
            if (sfEvent.type == sf::Event::MouseButtonPressed)
            {
                D2D::Mouse::MouseButton buttonPressed = ConvertSFMLMouseCode(sfEvent.mouseButton.button);
                D2D::MouseButtonPressedEvent *event = new D2D::MouseButtonPressedEvent(buttonPressed, sfEvent.mouseButton.x, sfEvent.mouseButton.y);
                D2D::EventBus::FireEvent<D2D::MouseButtonPressedEvent>(event);
            }
            if (sfEvent.type == sf::Event::MouseButtonReleased)
            {
                D2D::Mouse::MouseButton buttonPressed = ConvertSFMLMouseCode(sfEvent.mouseButton.button);
                D2D::MouseButtonReleasedEvent *event = new D2D::MouseButtonReleasedEvent(buttonPressed, sfEvent.mouseButton.x, sfEvent.mouseButton.y);
                D2D::EventBus::FireEvent<D2D::MouseButtonReleasedEvent>(event);
            }
            if (sfEvent.type == sf::Event::MouseMoved)
            {
                D2D::MouseMovedEvent *event = new D2D::MouseMovedEvent(sfEvent.mouseMove.x, sfEvent.mouseMove.y);
                D2D::EventBus::FireEvent<D2D::MouseMovedEvent>(event);
            }
            if (sfEvent.type == sf::Event::JoystickButtonPressed)
            {
                // D2D::KeyBoard::Keycode keyPressed = ConvertSFMLKeyCode(event.joystickButton.button);
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