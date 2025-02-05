#pragma once
#include "Core/App.h"
#include "Umbra.h"

extern Umbra::SharedPtr<Umbra::IGameInstance> CreateApplication();

int main() {

    Umbra::SharedPtr<Umbra::IGameInstance> gameInstance = std::move(CreateApplication());
    if (gameInstance != nullptr) {
        Umbra::UniquePtr<Umbra::App> app = std::make_unique<Umbra::App>(gameInstance);
        return app->Bootup();
    }
    return EXIT_FAILURE;
}
