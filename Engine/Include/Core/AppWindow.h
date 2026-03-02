#pragma once
#include "Core/Event.h"
#include "Graphics/IRenderDevice.h"
#include "Math/Vector.h"

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
        IRenderDevice* GetRenderDevice();
        bool bWindowClosed = true;

    private:
        void ResizeViewport(Umbra::Math::Vector2i& _resizedDeviceRes);

        UniquePtr<IRenderDevice> mRenderDevice;
        Math::Vector2i mScreenSize       = Math::Vector2i(0, 0);
        Math::Vector2i mRenderResolution = Math::Vector2i(0, 0);
        float aspectRatio                = 1;
    };

    class AppClosedEvent : public Umbra::Event {};

    class NativeWindowEvent : Event {
    public:
        NativeWindowEvent(void* _nativeEvent) : mNativeEvent(_nativeEvent) {}
        void* mNativeEvent;
    };
} // namespace Umbra
