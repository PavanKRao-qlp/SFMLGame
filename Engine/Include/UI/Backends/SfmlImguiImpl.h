#pragma once
#include "Core/AppWindow.h"
#include "UI/ImGuiBackend.h"

#include <SFML/Graphics.hpp>

namespace Umbra {
    class SfmlImguiImpl : public ImGuiBackend {
        bool Initialize(void* window, unsigned int windowWidth, unsigned int windowHeight) override;
        void ProcessEvent(void* event) override;
        void NewFrame(float deltaTime) override;
        void Render() override;
        void Shutdown() override;

    private:
        void OnSFMLEvent(const SFMLAppWindowEvent& _event);
        sf::RenderWindow* mWindow;
    };

} // namespace Umbra
