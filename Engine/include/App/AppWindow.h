#pragma once 
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

class AppWindow {
    public:
    AppWindow();
    ~AppWindow();
    bool CreateWindow();
    void Update();
    void RefreshDisplay();
    void ClearDisplay();
    void CloseWindow();
    bool bWindowClosed = true;
    private:
    sf::RenderWindow* mWindow;
};
