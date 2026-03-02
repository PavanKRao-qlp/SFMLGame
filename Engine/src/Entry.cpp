#pragma once
#include "Core/App.h"
#include "Umbra.h"

extern Umbra::SharedPtr<Umbra::IGameInstance> CreateApplication();

/// @brief  Entry point of the application for Windows
/// @return Exit Code
int main() {
    Umbra::SharedPtr<Umbra::IGameInstance> gameInstance = std::move(CreateApplication());
    if (gameInstance != nullptr) {
        Umbra::UniquePtr<Umbra::App> app = std::make_unique<Umbra::App>(gameInstance);
        return app->Bootup();
    }
    return EXIT_FAILURE;
}
