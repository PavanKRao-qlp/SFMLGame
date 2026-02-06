#pragma once
#include "EnginePCH.h"
#include "Graphics/Color.h"
#include "Graphics/RenderTypes.h"
#include "Math/Vector.h"

namespace Umbra {

    class IRenderDevice {
    public:
        virtual ~IRenderDevice() = default;

        // Window management
        virtual bool Create(int _width, int _height, const String& _title) = 0;
        virtual bool IsOpen() const                                        = 0;
        virtual void Close()                                               = 0;
        virtual Math::Vector2i GetWindowSize() const                       = 0;
        virtual void* GetNativeWindowHandle()                              = 0;

        // Display
        virtual void Clear(const Color& _color) = 0;
        virtual void Display()                   = 0;

        // Event polling
        virtual bool PollEvent()                              = 0;
        virtual EWindowEvent GetEventType() const             = 0;
        virtual int GetEventKeyCode() const                   = 0;
        virtual int GetEventMouseButton() const               = 0;
        virtual Math::Vector2i GetEventMousePosition() const  = 0;
        virtual Math::Vector2i GetEventResizeSize() const     = 0;
        virtual void* GetNativeEvent()                        = 0;

        // Camera / View
        virtual void SetViewSize(float _width, float _height)  = 0;
        virtual void SetViewCenter(float _x, float _y)         = 0;
        virtual void SetViewport(const FloatRect& _viewport)   = 0;
        virtual void ApplyView()                                = 0;

        // Coordinate conversion
        virtual Math::Vector2f MapPixelToCoords(Math::Vector2i _pixel) const = 0;

        // Drawing primitives
        virtual void DrawFilledRect(Math::Vector2f _position, Math::Vector2f _size,
            Math::Vector2f _origin, float _angle, const Color& _color) = 0;

        virtual void DrawTexturedRect(Math::Vector2f _position, Math::Vector2f _size,
            Math::Vector2f _origin, float _angle, const Color& _color, void* _textureHandle,
            const FloatRect& _uvRect = {0.f, 0.f, 1.f, 1.f}) = 0;

        // View bounds for frustum culling
        virtual FloatRect GetViewBounds() const = 0;

        virtual void DrawLine(Math::Vector2f _from, Math::Vector2f _to, const Color& _color) = 0;

        virtual void DrawCircle(Math::Vector2f _center, float _radius,
            bool _filled, const Color& _color) = 0;

        virtual void DrawRect(Math::Vector2f _position, Math::Vector2f _size,
            Math::Vector2f _origin, float _angle, bool _filled, const Color& _color) = 0;

        // Batched rendering
        virtual void BeginBatch() = 0;
        virtual void BatchQuad(Math::Vector2f _position, Math::Vector2f _size,
            Math::Vector2f _origin, float _angle, const Color& _color, void* _textureHandle,
            const FloatRect& _uvRect = {0.f, 0.f, 1.f, 1.f}) = 0;
        virtual void EndBatch() = 0;
    };

} // namespace Umbra
