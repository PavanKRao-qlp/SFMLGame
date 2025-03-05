#include "ECS/Component.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "Core/Random.h"
class PointObjectSpawnSystem : public Umbra::System
{
private:
    /* data */
public:
    inline PointObjectSpawnSystem(/* args */) : Umbra::System(new Umbra::ECView())
    {
        UMBRA_LOG_INFO("PointObjectSpawnSystem Created!");
        mSpawnCoolDown = mTimeToSpawn;
    };
    ~PointObjectSpawnSystem()
    {
        UMBRA_LOG_INFO("PointObjectSpawnSystem Destroyed!");
    }

    inline void Update() override
    {
        if (mSpawnCoolDown > 0)
        {
            mSpawnCoolDown -= Umbra::EngineTime::GetDeltaTime();
        }
        else
        {
            SpawnRandomObject();
            mSpawnCoolDown = mTimeToSpawn;
        }
    }

private:
    inline void SpawnRandomObject()
    {
        UMBRA_LOG_WARNING("Spawning Random Object!");
        Umbra::EntityID entity = mView->ecsRegister->CreateEntity();
        Umbra::Math::Vector2f RandomSize = Umbra::Math::Vector2f(Umbra::Random::RandomRange(10, 40), Umbra::Random::RandomRange(10, 40));
        mView->ecsRegister->AddComponent<Umbra::SpriteComponent>(entity, sf::Color::Red);
        mView->ecsRegister->AddComponent<Umbra::TransformComponent>(entity, Umbra::TransformComponent(
                                                                                Umbra::Math::Vector2f(0, 0),
                                                                                RandomSize));
        Umbra::PhysicsBodyComponent physicsBodyComponent = Umbra::PhysicsBodyComponent();
        physicsBodyComponent.SetMass(1);
        Umbra::Math::Vector2f RandomVelocity = Umbra::Math::Vector2f(Umbra::Random::RandomRange(-50, 50), Umbra::Random::RandomRange(-50, 50));
        physicsBodyComponent.mVelocity = RandomVelocity;
        mView->ecsRegister->AddComponent<Umbra::PhysicsBodyComponent>(entity, physicsBodyComponent);
    }
    float mSpawnCoolDown = 0;
    float mTimeToSpawn = .75f;
};
