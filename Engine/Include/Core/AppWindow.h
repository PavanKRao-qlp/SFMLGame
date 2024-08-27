#pragma once 
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "Event.h"

class AppWindow {
    public:
    AppWindow();
    ~AppWindow();
    bool CreateWindow();
    void Update();
    void RefreshDisplay();
    void ClearDisplay();
    void CloseWindow();
    sf::RenderWindow*  GetRenderWindowHandle();
    bool bWindowClosed = true;
    private:
    sf::RenderWindow* mWindow;
};

class AppClosedEvent : public D2D::Event {};