#pragma once
#include "App/AppWindow.h"
#include "Log/Logger.h"

AppWindow::AppWindow()
{
}

AppWindow::~AppWindow()
{
    if(mWindow)
        delete mWindow;
}

bool AppWindow::CreateWindow()
{
  Logger::Log(LogType::Verbose, "Creating Window");
    mWindow = new sf::RenderWindow(sf::VideoMode(800, 800), "My window");
    if (mWindow == nullptr) {
        Logger::Log(LogType::FatalError, "Creating Window Failed");
        return false;
    }
    bWindowClosed = false;
    return true;
};

void AppWindow::Update()
{  // run the program as long as the window is open
    if (mWindow->isOpen())
    {
        sf::Event event;
        while (mWindow->pollEvent(event))
        {
    // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                 bWindowClosed = true;
        }
    }
}

void AppWindow::RefreshDisplay()
{
    sf::RectangleShape rect;
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
    if(mWindow->isOpen())
        mWindow->close();
}
