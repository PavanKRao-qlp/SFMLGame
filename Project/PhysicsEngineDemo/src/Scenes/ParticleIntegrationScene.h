#pragma once
#include "Game/Scene.h"
#include "Math/Vector.h"

class ParticleIntegrationScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    void SpawnParticle();
    void DrawPhysicsWorldConfigWidget();

    // Spawn config
    float mSpawnInterval    = 0.5f; // seconds between spawns
    float mSpawnTimer       = 0.0f;
    bool bAutoSpawn         = true;
    int mParticlesPerSpawn  = 1;
    float mParticleLifetime = 5.0f;

    // Particle config
    float mMinSpeed        = 5.0f;
    float mMaxSpeed        = 30.0f;
    float mMinAngularSpeed = 0.0f;
    float mMaxAngularSpeed = 0.0f;
    float mParticleSize    = 5.0f;
    float mParticleMass    = 1.0f;
};
