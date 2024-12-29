#pragma once
#include "Event.h"
#include <Math/Vector.h>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace Umbra {
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
        void ResizeViewport(Umbra::Math::Vector2i& resizedDeviceRes);

        sf::RenderWindow* mWindow;
        sf::View* mView;
        Math::Vector2i mScreenSize       = Math::Vector2i(0, 0);
        Math::Vector2i mRenderResolution = Math::Vector2i(0, 0);
        float aspectRatio                = 1;
    };

    class AppClosedEvent : public Umbra::Event {};

} // namespace Umbra
