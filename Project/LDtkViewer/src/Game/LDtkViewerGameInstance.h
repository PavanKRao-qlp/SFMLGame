#pragma once
#include "Game/IGameInstance.h"

class LDtkViewerGameInstance : public Umbra::IGameInstance {
public:
    LDtkViewerGameInstance();
    ~LDtkViewerGameInstance();
    void Initialize() override;
    void ShutDown()   override;
};
