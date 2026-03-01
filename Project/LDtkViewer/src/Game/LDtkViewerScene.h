#pragma once
#include "EnginePCH.h"
#include "LDtk/LDtkScene.h"

class LDtkViewerScene : public Umbra::LDtkScene {
public:
    LDtkViewerScene(const Umbra::String& _ldtkPath, const Umbra::String& _levelName = "");

    void                           Initialize()       override;
    void                           OnUpdate()         override;
    Umbra::SharedPtr<Umbra::Scene> InstantiateCopy()  override;

private:
    Umbra::String mPath;
    Umbra::String mLevel;
};
