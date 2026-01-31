#pragma once
#include "Core/AppWindow.h"

#include "Graphics/Backends/SfmlRenderDevice.h"
#include "Input/Input.h"
#include "Umbra.h"

namespace Umbra {
    AppWindow::AppWindow() {
        GEngineStatics.AppWindowPtr = this;
    }

    AppWindow::~AppWindow() {
        mRenderDevice.reset();
    }

    bool AppWindow::CreateWindow() {
        UMBRA_LOG_INFO("Creating Window");
        mScreenSize       = Math::Vector2i(800, 800);
        mRenderResolution = Math::Vector2i(400, 400);
        aspectRatio       = ((float) mRenderResolution.x) / mRenderResolution.y;

        mRenderDevice = std::make_unique<SfmlRenderDevice>();
        bool success  = mRenderDevice->Create(mScreenSize.x, mScreenSize.y, "My window");

        bWindowClosed = false;
        return success;
    }

    void AppWindow::Update() {
        if (!mRenderDevice || !mRenderDevice->IsOpen()) return;

        while (mRenderDevice->PollEvent()) {
            EWindowEvent eventType = mRenderDevice->GetEventType();

            if (eventType == EWindowEvent::Closed) {
                bWindowClosed = true;
                Umbra::EventBus::FireEvent<AppClosedEvent>(AppClosedEvent());
            }
            if (eventType == EWindowEvent::KeyPressed) {
                Umbra::KeyBoard::Keycode keyPressed = static_cast<Umbra::KeyBoard::Keycode>(mRenderDevice->GetEventKeyCode());
                Umbra::EventBus::FireEvent<Umbra::KeyPressedEvent>(Umbra::KeyPressedEvent(keyPressed));
            }
            if (eventType == EWindowEvent::KeyReleased) {
                Umbra::KeyBoard::Keycode keyPressed = static_cast<Umbra::KeyBoard::Keycode>(mRenderDevice->GetEventKeyCode());
                Umbra::EventBus::FireEvent<Umbra::KeyReleasedEvent>(Umbra::KeyReleasedEvent(keyPressed));
            }
            if (eventType == EWindowEvent::MouseButtonPressed) {
                Umbra::Mouse::MouseButton buttonPressed =
                    static_cast<Umbra::Mouse::MouseButton>(mRenderDevice->GetEventMouseButton());
                Math::Vector2i mousePos = mRenderDevice->GetEventMousePosition();
                Umbra::EventBus::FireEvent<Umbra::MouseButtonPressedEvent>(
                    Umbra::MouseButtonPressedEvent(buttonPressed, mousePos.x, mousePos.y));
            }
            if (eventType == EWindowEvent::MouseButtonReleased) {
                Umbra::Mouse::MouseButton buttonPressed =
                    static_cast<Umbra::Mouse::MouseButton>(mRenderDevice->GetEventMouseButton());
                Math::Vector2i mousePos = mRenderDevice->GetEventMousePosition();
                Umbra::EventBus::FireEvent<Umbra::MouseButtonReleasedEvent>(
                    Umbra::MouseButtonReleasedEvent(buttonPressed, mousePos.x, mousePos.y));
            }
            if (eventType == EWindowEvent::MouseMoved) {
                Math::Vector2i mousePos = mRenderDevice->GetEventMousePosition();
                Umbra::EventBus::FireEvent<Umbra::MouseMovedEvent>(Umbra::MouseMovedEvent(mousePos.x, mousePos.y));
            }
            if (eventType == EWindowEvent::Resized) {
                Math::Vector2i resizedDeviceRes = mRenderDevice->GetEventResizeSize();
                ResizeViewport(resizedDeviceRes);
            }

            // Fire native event for backends that need it (e.g., ImGui-SFML)
            Umbra::EventBus::FireEvent<Umbra::NativeWindowEvent>(
                Umbra::NativeWindowEvent(mRenderDevice->GetNativeEvent()));
        }
    }

    void AppWindow::ResizeViewport(Umbra::Math::Vector2i& _resizedDeviceRes) {
    }

    void AppWindow::RefreshDisplay() {
        mRenderDevice->Display();
    }

    void AppWindow::ClearDisplay() {
        mRenderDevice->Clear(Color::Red);
    }

    void AppWindow::CloseWindow() {
        UMBRA_LOG_INFO("Window Closed");
        if (mRenderDevice && mRenderDevice->IsOpen()) {
            mRenderDevice->Close();
        }
    }

    IRenderDevice* AppWindow::GetRenderDevice() {
        return mRenderDevice.get();
    }
} // namespace Umbra
