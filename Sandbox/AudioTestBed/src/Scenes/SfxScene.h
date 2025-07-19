#pragma once
#include "Game/Scene.h"

#include <SFML/Audio.hpp>
class SfxScene : public Umbra::Scene {

public:
    virtual void Initialize() override;
    virtual void OnFixedUpdated() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InsatiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    float mSfxVolume = 0;
    sf::SoundBuffer buffer;
    sf::Sound sound1;
};
