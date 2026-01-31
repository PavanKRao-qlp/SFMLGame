#pragma once
#include "Graphics/IRenderDevice.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace Umbra {

    class SfmlRenderDevice : public IRenderDevice {
    public:
        SfmlRenderDevice();
        ~SfmlRenderDevice();

        // Window management
        bool Create(int _width, int _height, const String& _title) override;
        bool IsOpen() const override;
        void Close() override;
        Math::Vector2i GetWindowSize() const override;
        void* GetNativeWindowHandle() override;

        // Display
        void Clear(const Color& _color) override;
        void Display() override;

        // Event polling
        bool PollEvent() override;
        EWindowEvent GetEventType() const override;
        int GetEventKeyCode() const override;
        int GetEventMouseButton() const override;
        Math::Vector2i GetEventMousePosition() const override;
        Math::Vector2i GetEventResizeSize() const override;
        void* GetNativeEvent() override;

        // Camera / View
        void SetViewSize(float _width, float _height) override;
        void SetViewCenter(float _x, float _y) override;
        void SetViewport(const FloatRect& _viewport) override;
        void ApplyView() override;

        // Coordinate conversion
        Math::Vector2f MapPixelToCoords(Math::Vector2i _pixel) const override;

        // Drawing primitives
        void DrawFilledRect(Math::Vector2f _position, Math::Vector2f _size,
            Math::Vector2f _origin, float _angle, const Color& _color) override;

        void DrawTexturedRect(Math::Vector2f _position, Math::Vector2f _size,
            Math::Vector2f _origin, float _angle, const Color& _color, void* _textureHandle) override;

        void DrawLine(Math::Vector2f _from, Math::Vector2f _to, const Color& _color) override;

        void DrawCircle(Math::Vector2f _center, float _radius,
            bool _filled, const Color& _color) override;

        void DrawRect(Math::Vector2f _position, Math::Vector2f _size,
            Math::Vector2f _origin, float _angle, bool _filled, const Color& _color) override;

    private:
        static sf::Color ToSfColor(const Color& _color);

        sf::RenderWindow* mWindow = nullptr;
        sf::View* mView           = nullptr;
        sf::Event mCurrentEvent;
    };

} // namespace Umbra
