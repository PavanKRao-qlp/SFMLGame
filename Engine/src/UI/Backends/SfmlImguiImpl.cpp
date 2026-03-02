#include "UI/Backends/SfmlImguiImpl.h"

#include "Core/Event.h"
#include "imgui.h"
#include <imgui-SFML.h>

namespace Umbra {
    bool SfmlImguiImpl::Init(void* _window, unsigned int _windowWidth, unsigned int _windowHeight) {
        mWindow = static_cast<sf::RenderWindow*>(_window);
        if (mWindow) {
            EventBus::Subscribe<NativeWindowEvent>(BIND_1P(this, &SfmlImguiImpl::OnNativeWindowEvent));
            return ImGui::SFML::Init(*mWindow);
        }
        return false;
    }

    void SfmlImguiImpl::OnNativeWindowEvent(const NativeWindowEvent& _event) {
        if (_event.mNativeEvent) {
            ProcessEvent(_event.mNativeEvent);
        }
    }

    void SfmlImguiImpl::ProcessEvent(void* _event) {
        ImGui::SFML::ProcessEvent(*mWindow, *static_cast<sf::Event*>(_event));
    }

    void SfmlImguiImpl::NewFrame(float _deltaTime) {
        ImGui::SFML::Update(*mWindow, sf::seconds(_deltaTime));
    }

    void SfmlImguiImpl::Render() {
        ImGui::SFML::Render(*mWindow);
    }

    void SfmlImguiImpl::Shutdown() {
        ImGui::SFML::Shutdown();
    }

} // namespace Umbra
