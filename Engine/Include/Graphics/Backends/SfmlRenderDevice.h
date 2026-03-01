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
            Math::Vector2f _origin, float _angle, const Color& _color, void* _textureHandle,
            const FloatRect& _uvRect = {0.f, 0.f, 1.f, 1.f}) override;

        FloatRect GetViewBounds() const override;

        void DrawLine(Math::Vector2f _from, Math::Vector2f _to, const Color& _color) override;

        void DrawCircle(Math::Vector2f _center, float _radius,
            bool _filled, const Color& _color) override;

        void DrawRect(Math::Vector2f _position, Math::Vector2f _size,
            Math::Vector2f _origin, float _angle, bool _filled, const Color& _color) override;

        // Batched rendering
        void BeginBatch() override;
        void BatchQuad(Math::Vector2f _position, Math::Vector2f _size,
            Math::Vector2f _origin, float _angle, const Color& _color, void* _textureHandle,
            const FloatRect& _uvRect = {0.f, 0.f, 1.f, 1.f}, void* _shaderHandle = nullptr) override;
        void EndBatch() override;

    private:
        static sf::Color ToSfColor(const Color& _color);

        void AddQuadVertices(sf::VertexArray& _vertices, Math::Vector2f _position, Math::Vector2f _size,
            Math::Vector2f _origin, float _angle, const Color& _color, const sf::Texture* _texture,
            const FloatRect& _uvRect);

        sf::RenderWindow* mWindow = nullptr;
        sf::View* mView           = nullptr;
        sf::Event mCurrentEvent;

        // Batching state
        bool mBatching = false;
        struct BatchData {
            sf::VertexArray    vertices;
            const sf::Texture* texture = nullptr;
            const sf::Shader*  shader  = nullptr;
        };
        Vector<BatchData> mBatchList;
        void* mCurrentBatchTexture = nullptr;
        void* mCurrentBatchShader  = nullptr;
    };

} // namespace Umbra
