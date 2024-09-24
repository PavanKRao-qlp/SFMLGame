#pragma once
#include "Event.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class AppWindow {
public:
    AppWindow();
    ~AppWindow();
    bool CreateWindow();
    void Update();
    void RefreshDisplay();
    void ClearDisplay();
    void CloseWindow();
    sf::RenderWindow* GetRenderWindowHandle();
    bool bWindowClosed = true;

private:
    sf::RenderWindow* mWindow;
};

class AppClosedEvent : public Umbra::Event {};
