#pragma once
#include "Core/AppWindow.h"
#include "Diag/Logger.h"

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
        sf::Event event;
        while (mWindow->pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed){
                bWindowClosed = true;
                D2D::EventBus::FireEvent<AppClosedEvent>(new AppClosedEvent());
            }
        }
    }
}

void AppWindow::RefreshDisplay()
{
    sf::RectangleShape rect;
    rect.setPosition(400, 400);
    rect.setSize(sf::Vector2f(100, 100));
    rect.setFillColor(sf::Color::Red);
    mWindow->draw(rect);
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
