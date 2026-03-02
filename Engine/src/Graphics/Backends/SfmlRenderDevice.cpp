#include "Graphics/Backends/SfmlRenderDevice.h"

namespace Umbra {

    SfmlRenderDevice::SfmlRenderDevice() {
        mCurrentEvent = sf::Event();
    }

    SfmlRenderDevice::~SfmlRenderDevice() {
        if (mView) {
            delete mView;
            mView = nullptr;
        }
        if (mWindow) {
            delete mWindow;
            mWindow = nullptr;
        }
    }

    bool SfmlRenderDevice::Create(int _width, int _height, const String& _title) {
        mWindow = new sf::RenderWindow(sf::VideoMode(_width, _height), _title);
        mView   = new sf::View(sf::Vector2f(0, 0), sf::Vector2f(static_cast<float>(_width), static_cast<float>(_height)));
        return (mWindow != nullptr);
    }

    bool SfmlRenderDevice::IsOpen() const {
        return mWindow && mWindow->isOpen();
    }

    void SfmlRenderDevice::Close() {
        if (mWindow && mWindow->isOpen()) {
            mWindow->close();
        }
    }

    Math::Vector2i SfmlRenderDevice::GetWindowSize() const {
        if (!mWindow) return Math::Vector2i(0, 0);
        sf::Vector2u size = mWindow->getSize();
        return Math::Vector2i(static_cast<int>(size.x), static_cast<int>(size.y));
    }

    void* SfmlRenderDevice::GetNativeWindowHandle() {
        return mWindow;
    }

    // Display

    void SfmlRenderDevice::Clear(const Color& _color) {
        if (mWindow) {
            mWindow->clear(ToSfColor(_color));
        }
    }

    void SfmlRenderDevice::Display() {
        if (mWindow) {
            mWindow->display();
        }
    }

    // Event polling

    bool SfmlRenderDevice::PollEvent() {
        if (!mWindow) return false;
        return mWindow->pollEvent(mCurrentEvent);
    }

    EWindowEvent SfmlRenderDevice::GetEventType() const {
        switch (mCurrentEvent.type) {
        case sf::Event::Closed:             return EWindowEvent::Closed;
        case sf::Event::KeyPressed:         return EWindowEvent::KeyPressed;
        case sf::Event::KeyReleased:        return EWindowEvent::KeyReleased;
        case sf::Event::MouseButtonPressed: return EWindowEvent::MouseButtonPressed;
        case sf::Event::MouseButtonReleased:return EWindowEvent::MouseButtonReleased;
        case sf::Event::MouseMoved:         return EWindowEvent::MouseMoved;
        case sf::Event::JoystickButtonPressed: return EWindowEvent::JoystickButtonPressed;
        case sf::Event::Resized:            return EWindowEvent::Resized;
        default:                            return EWindowEvent::None;
        }
    }

    int SfmlRenderDevice::GetEventKeyCode() const {
        return static_cast<int>(mCurrentEvent.key.code);
    }

    int SfmlRenderDevice::GetEventMouseButton() const {
        return static_cast<int>(mCurrentEvent.mouseButton.button);
    }

    Math::Vector2i SfmlRenderDevice::GetEventMousePosition() const {
        if (mCurrentEvent.type == sf::Event::MouseMoved) {
            return Math::Vector2i(mCurrentEvent.mouseMove.x, mCurrentEvent.mouseMove.y);
        }
        return Math::Vector2i(mCurrentEvent.mouseButton.x, mCurrentEvent.mouseButton.y);
    }

    Math::Vector2i SfmlRenderDevice::GetEventResizeSize() const {
        return Math::Vector2i(
            static_cast<int>(mCurrentEvent.size.width),
            static_cast<int>(mCurrentEvent.size.height));
    }

    void* SfmlRenderDevice::GetNativeEvent() {
        return &mCurrentEvent;
    }

    // Camera / View

    void SfmlRenderDevice::SetViewSize(float _width, float _height) {
        if (mView) {
            mView->setSize(_width, _height);
        }
    }

    void SfmlRenderDevice::SetViewCenter(float _x, float _y) {
        if (mView) {
            mView->setCenter(_x, _y);
        }
    }

    void SfmlRenderDevice::SetViewport(const FloatRect& _viewport) {
        if (mView) {
            mView->setViewport(sf::FloatRect(_viewport.Left, _viewport.Top, _viewport.Width, _viewport.Height));
        }
    }

    void SfmlRenderDevice::ApplyView() {
        if (mWindow && mView) {
            mWindow->setView(*mView);
        }
    }

    // Coordinate conversion

    Math::Vector2f SfmlRenderDevice::MapPixelToCoords(Math::Vector2i _pixel) const {
        if (!mWindow) return Math::Vector2f(0.f, 0.f);
        sf::Vector2f worldPos = mWindow->mapPixelToCoords(sf::Vector2i(_pixel.x, _pixel.y));
        return Math::Vector2f(worldPos.x, worldPos.y);
    }

    // Drawing primitives

    void SfmlRenderDevice::DrawFilledRect(Math::Vector2f _position, Math::Vector2f _size,
        Math::Vector2f _origin, float _angle, const Color& _color) {
        if (!mWindow) return;
        sf::RectangleShape shape;
        shape.setSize(sf::Vector2f(_size.x, _size.y));
        shape.setOrigin(_origin.x * _size.x, _origin.y * _size.y);
        shape.setPosition(sf::Vector2f(_position.x, _position.y));
        shape.setRotation(_angle);
        shape.setFillColor(ToSfColor(_color));
        mWindow->draw(shape);
    }

    void SfmlRenderDevice::DrawTexturedRect(Math::Vector2f _position, Math::Vector2f _size,
        Math::Vector2f _origin, float _angle, const Color& _color, void* _textureHandle,
        const FloatRect& _uvRect) {
        if (!mWindow) return;
        sf::RectangleShape shape;
        shape.setSize(sf::Vector2f(_size.x, _size.y));
        shape.setOrigin(_origin.x * _size.x, _origin.y * _size.y);
        shape.setPosition(sf::Vector2f(_position.x, _position.y));
        shape.setRotation(_angle);
        shape.setFillColor(ToSfColor(_color));
        if (_textureHandle) {
            const sf::Texture* texture = static_cast<const sf::Texture*>(_textureHandle);
            shape.setTexture(texture);
            sf::Vector2u texSize = texture->getSize();
            sf::IntRect texRect(
                static_cast<int>(_uvRect.Left * texSize.x),
                static_cast<int>(_uvRect.Top * texSize.y),
                static_cast<int>(_uvRect.Width * texSize.x),
                static_cast<int>(_uvRect.Height * texSize.y));
            shape.setTextureRect(texRect);
        }
        mWindow->draw(shape);
    }

    void SfmlRenderDevice::DrawLine(Math::Vector2f _from, Math::Vector2f _to, const Color& _color) {
        if (!mWindow) return;
        sf::VertexArray lines(sf::LinesStrip, 2);
        lines[0].position = sf::Vector2f(_from.x, _from.y);
        lines[0].color    = ToSfColor(_color);
        lines[1].position = sf::Vector2f(_to.x, _to.y);
        lines[1].color    = ToSfColor(_color);
        mWindow->draw(lines);
    }

    void SfmlRenderDevice::DrawCircle(Math::Vector2f _center, float _radius,
        bool _filled, const Color& _color) {
        if (!mWindow) return;
        sf::CircleShape shape(_radius);
        shape.setPosition(sf::Vector2f(_center.x - _radius, _center.y - _radius));
        if (_filled) {
            shape.setFillColor(ToSfColor(_color));
        } else {
            shape.setOutlineThickness(0.2f);
            shape.setOutlineColor(ToSfColor(_color));
            shape.setFillColor(sf::Color::Transparent);
        }
        mWindow->draw(shape);
    }

    void SfmlRenderDevice::DrawRect(Math::Vector2f _position, Math::Vector2f _size,
        Math::Vector2f _origin, float _angle, bool _filled, const Color& _color) {
        if (!mWindow) return;
        sf::RectangleShape shape;
        shape.setSize(sf::Vector2f(_size.x, _size.y));
        shape.setOrigin(sf::Vector2f(_origin.x, _origin.y));
        shape.setPosition(sf::Vector2f(_position.x, _position.y));
        shape.setRotation(_angle);
        if (_filled) {
            shape.setFillColor(ToSfColor(_color));
        } else {
            shape.setOutlineThickness(0.5f);
            shape.setOutlineColor(ToSfColor(_color));
            shape.setFillColor(sf::Color::Transparent);
        }
        mWindow->draw(shape);
    }

    FloatRect SfmlRenderDevice::GetViewBounds() const {
        if (!mView) return FloatRect(0.f, 0.f, 0.f, 0.f);
        sf::Vector2f center = mView->getCenter();
        sf::Vector2f size = mView->getSize();
        return FloatRect(
            center.x - size.x * 0.5f,
            center.y - size.y * 0.5f,
            size.x,
            size.y);
    }

    // Batched rendering

    void SfmlRenderDevice::BeginBatch() {
        mBatching = true;
        mBatchList.clear();
        mCurrentBatchTexture = reinterpret_cast<void*>(~uintptr_t(0)); // sentinel
        mCurrentBatchShader  = reinterpret_cast<void*>(~uintptr_t(0)); // sentinel
    }

    void SfmlRenderDevice::BatchQuad(Math::Vector2f _position, Math::Vector2f _size,
        Math::Vector2f _origin, float _angle, const Color& _color, void* _textureHandle,
        const FloatRect& _uvRect, void* _shaderHandle) {
        if (!mBatching) return;

        // Start a new batch when either the texture or the shader changes
        if (_textureHandle != mCurrentBatchTexture || _shaderHandle != mCurrentBatchShader
            || mBatchList.empty()) {
            mCurrentBatchTexture = _textureHandle;
            mCurrentBatchShader  = _shaderHandle;
            BatchData data;
            data.vertices.setPrimitiveType(sf::Quads);
            data.texture = static_cast<const sf::Texture*>(_textureHandle);
            data.shader  = static_cast<const sf::Shader*>(_shaderHandle);
            mBatchList.push_back(std::move(data));
        }

        AddQuadVertices(mBatchList.back().vertices, _position, _size, _origin, _angle, _color,
            static_cast<const sf::Texture*>(_textureHandle), _uvRect);
    }

    void SfmlRenderDevice::EndBatch() {
        if (!mWindow || !mBatching) return;
        mBatching = false;

        for (auto& batch : mBatchList) {
            if (batch.vertices.getVertexCount() == 0) continue;

            sf::RenderStates states;
            if (batch.texture) states.texture = batch.texture;
            if (batch.shader)  states.shader  = batch.shader;
            mWindow->draw(batch.vertices, states);
        }
    }

    void SfmlRenderDevice::AddQuadVertices(sf::VertexArray& _vertices, Math::Vector2f _position, Math::Vector2f _size,
        Math::Vector2f _origin, float _angle, const Color& _color, const sf::Texture* _texture,
        const FloatRect& _uvRect) {

        sf::Color sfColor = ToSfColor(_color);

        // Calculate corner positions relative to origin
        float ox = _origin.x * _size.x;
        float oy = _origin.y * _size.y;

        // Corners before rotation (relative to origin)
        sf::Vector2f corners[4] = {
            sf::Vector2f(-ox, -oy),                           // Top-left
            sf::Vector2f(_size.x - ox, -oy),                  // Top-right
            sf::Vector2f(_size.x - ox, _size.y - oy),         // Bottom-right
            sf::Vector2f(-ox, _size.y - oy)                   // Bottom-left
        };

        // Apply rotation
        float rad = _angle * 3.14159265f / 180.f;
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);

        for (int i = 0; i < 4; ++i) {
            float rx = corners[i].x * cosA - corners[i].y * sinA;
            float ry = corners[i].x * sinA + corners[i].y * cosA;
            corners[i] = sf::Vector2f(_position.x + rx, _position.y + ry);
        }

        // Calculate texture coordinates
        sf::Vector2f texCoords[4];
        if (_texture) {
            sf::Vector2u texSize = _texture->getSize();
            float left = _uvRect.Left * texSize.x;
            float top = _uvRect.Top * texSize.y;
            float right = (_uvRect.Left + _uvRect.Width) * texSize.x;
            float bottom = (_uvRect.Top + _uvRect.Height) * texSize.y;
            texCoords[0] = sf::Vector2f(left, top);
            texCoords[1] = sf::Vector2f(right, top);
            texCoords[2] = sf::Vector2f(right, bottom);
            texCoords[3] = sf::Vector2f(left, bottom);
        }

        // Add vertices
        for (int i = 0; i < 4; ++i) {
            sf::Vertex vertex;
            vertex.position = corners[i];
            vertex.color = sfColor;
            if (_texture) {
                vertex.texCoords = texCoords[i];
            }
            _vertices.append(vertex);
        }
    }

    // Utility

    sf::Color SfmlRenderDevice::ToSfColor(const Color& _color) {
        return sf::Color(_color.r8(), _color.g8(), _color.b8(), _color.a8());
    }

} // namespace Umbra
