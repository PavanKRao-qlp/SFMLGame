#pragma once
#include "Core/AppWindow.h"

#include "Input/Input.h"
#include "Umbra.h"
#include "sfmlHelper.h"
namespace Umbra {
    AppWindow::AppWindow() {}

    AppWindow::~AppWindow() {
        if (mWindow) {
            delete mWindow;
        }
    }

    bool AppWindow::CreateWindow() {
        Logger::Log(LogType::Verbose, "Creating Window");
        mScreenSize       = (800, 800);
        mRenderResolution = (400, 400);
        aspectRatio       = ((float) mRenderResolution.x) / mRenderResolution.y;
        mWindow           = new sf::RenderWindow(sf::VideoMode(mScreenSize.x, mScreenSize.y), "My window");
        mView             = new sf::View(sf::Vector2f(0, 0), sf::Vector2f(mRenderResolution.x, mRenderResolution.y));
        ResizeViewport(mScreenSize);
        UM_ASSERT(mWindow != nullptr, "Creating Window Failed");
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
                    Umbra::MouseButtonReleasedEvent* event  = new Umbra::MouseButtonReleasedEvent(
                        buttonPressed, sfEvent.mouseButton.x, sfEvent.mouseButton.y);
                    Umbra::EventBus::FireEvent<Umbra::MouseButtonReleasedEvent>(event);
                }
                if (sfEvent.type == sf::Event::MouseMoved) {
                    Umbra::MouseMovedEvent* event =
                        new Umbra::MouseMovedEvent(sfEvent.mouseMove.x, sfEvent.mouseMove.y);
                    Umbra::EventBus::FireEvent<Umbra::MouseMovedEvent>(event);
                }
                if (sfEvent.type == sf::Event::JoystickButtonPressed) {
                    // Umbra::KeyBoard::Keycode keyPressed = ConvertSFMLKeyCode(event.joystickButton.button);
                }
                if (sfEvent.type == sf::Event::Resized) {
                    Math::Vector2i resizedDeviceRes = Math::Vector2i(sfEvent.size.width, sfEvent.size.height);
                    ResizeViewport(resizedDeviceRes);
                }
            }
        }
    }


    void AppWindow::ResizeViewport(Umbra::Math::Vector2i& resizedDeviceRes) {
        float newAspectRatio = (float) resizedDeviceRes.x / resizedDeviceRes.y;
        sf::FloatRect ResizeViewport(0, 0, 1, 1);
        if (newAspectRatio > aspectRatio) {
            ResizeViewport.width = aspectRatio / newAspectRatio;
            ResizeViewport.left  = (1 - ResizeViewport.width) / 2.f;
        } else {
            ResizeViewport.height = newAspectRatio / aspectRatio;
            ResizeViewport.top    = (1 - ResizeViewport.height) / 2.f;
            // resizedWidth  = 1;
        }

        mView->setViewport(ResizeViewport);
        mView->setCenter(sf::Vector2f(0, 0));
        mWindow->setView(*mView);
    }
    void AppWindow::RefreshDisplay() {
        mWindow->display();
    }

    void AppWindow::ClearDisplay() {
        mWindow->clear(sf::Color::Red);
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

} // namespace Umbra
