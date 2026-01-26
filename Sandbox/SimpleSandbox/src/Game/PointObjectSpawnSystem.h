#include "Core/Random.h"
#include "ECS/Component.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
class PointObjectSpawnSystem : public Umbra::System {
private:
    /* data */
public:
    inline PointObjectSpawnSystem(/* args */) : Umbra::System(std::make_unique<Umbra::ECView<>>()) {
        UMBRA_LOG_INFO("PointObjectSpawnSystem Created!");
        mSpawnCoolDown = mTimeToSpawn;
    }
    ~PointObjectSpawnSystem() {
        UMBRA_LOG_INFO("PointObjectSpawnSystem Destroyed!");
    }

    inline void Update() override {
        if (mSpawnCoolDown > 0) {
            mSpawnCoolDown -= Umbra::EngineTime::GetDeltaTime();
        } else {
            SpawnRandomObject();
            mSpawnCoolDown = mTimeToSpawn;
        }
    }

private:
    inline void SpawnRandomObject() {
        if (bStop) {
            return;
        }
        Umbra::EntityID entity = mView->ecsRegister->CreateEntity();
        if (entity > 25) {
            bStop = true;
        }
        UMBRA_LOG_DEBUG("Spawning Random Object! %llu", entity);
        Umbra::Math::Vector2f RandomSize =
            Umbra::Math::Vector2f(Umbra::Random::RandomRange(10, 40), Umbra::Random::RandomRange(10, 40));
        // mView->ecsRegister->AddComponent<Umbra::SpriteComponent>(entity, sf::Color::Red);
        mView->ecsRegister->AddComponent<Umbra::TransformComponent>(entity,
            Umbra::TransformComponent(
                Umbra::Math::Vector2f(Umbra::Random::RandomRange(-100, 100), Umbra::Random::RandomRange(-100, 100)),
                RandomSize));
        Umbra::PhysicsBodyComponent physicsBodyComponent = Umbra::PhysicsBodyComponent();
        physicsBodyComponent.SetMass(Umbra::Random::RandomRange(1, 10));
        physicsBodyComponent.bAffectedByGravity = false;
        Umbra::Math::Vector2f RandomVelocity =
            Umbra::Math::Vector2f(Umbra::Random::RandomRange(-50, 50), Umbra::Random::RandomRange(-50, 50));
        // physicsBodyComponent.mVelocity = RandomVelocity;
        mView->ecsRegister->AddComponent<Umbra::PhysicsBodyComponent>(entity, physicsBodyComponent);

        Umbra::CircleColliderComponent circleColliderComponent;
        circleColliderComponent.Offset = Umbra::Math::Vector2f(0, 0);
        circleColliderComponent.Radius = RandomSize.Magnitude() / 2;
        mView->ecsRegister->AddComponent<Umbra::CircleColliderComponent>(entity, circleColliderComponent);
    }
    float mSpawnCoolDown = 0;
    float mTimeToSpawn   = 1.5f;
    bool bStop           = false;
};
