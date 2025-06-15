#pragma once
#include "ECS/Components/Transfrom.h"
#include "Umbra.h"

namespace Umbra {
    class IForceGenerator {
    private:
    public:
        virtual void ApplyForce(float _deltaTime)        = 0;
        virtual bool IsForceEnactingOn(EntityID _entity) = 0;
        void ProvideECSView(BaseView* _view) {
            mView = _view;
        }

    protected:
        BaseView& GetECSView() {
            return *mView;
        }

    private:
        BaseView* mView;
    };


    class ITorqueGenerator {
    private:
    public:
        virtual void ApplyTorque(float _deltaTime)        = 0;
        virtual bool IsTorqueEnactingOn(EntityID _entity) = 0;
        void ProvideECSView(BaseView* _view) {
            mView = _view;
        }

    protected:
        BaseView& GetECSView() {
            return *mView;
        }

    private:
        BaseView* mView;
    };


    class SpringForceGenerator : public IForceGenerator {
    public:
        SpringForceGenerator(EntityID _entityA, EntityID _entityB, float _hookConstant, float _restLength)
            : mEntityA(_entityA), mEntityB(_entityB), mHooksConstant(_hookConstant), mRestLength(_restLength) {}

        virtual void ApplyForce(float _deltaTime) override {
            Math::Vector2f positionA = GetECSView().ecsRegister->GetComponent<TransformComponent>(mEntityA)->Position;
            Math::Vector2f positionB = GetECSView().ecsRegister->GetComponent<TransformComponent>(mEntityB)->Position;
            // RenderSystem::DebugDrawLine(positionA, positionB);
            Math::Vector2f length = (positionB - positionA);
            if (length.Magnitude() - mRestLength == 0) {
                return;
            }
            // RenderSystem::DebugDrawLine(
            //     positionB, positionB - length.GetNormalized() * mRestLength, sf::Color::Magenta);
            Math::Vector2f force = length.GetNormalized() * (-mHooksConstant * (mRestLength - length.Magnitude()));
            // RenderSystem::DebugDrawLine(positionA, positionA + force);
            //   UMBRA_LOG_DEBUG("force applied %f %f ", length.Magnitude() - mRestLength, force.Magnitude());
            PhysicsBodyComponent* bodyA = GetECSView().ecsRegister->GetComponent<PhysicsBodyComponent>(mEntityA);
            PhysicsBodyComponent* bodyB = GetECSView().ecsRegister->GetComponent<PhysicsBodyComponent>(mEntityB);
            if (bodyA->mInverseMass > 0) {
                bodyA->ApplyForce(force);
            }
            if (bodyB->mInverseMass > 0) {
                bodyB->ApplyForce(force * -1);
            }
        }

        virtual bool IsForceEnactingOn(EntityID _entity) override {
            if (_entity == mEntityB || _entity == mEntityA) {
                return true;
            }
            return false;
        }

    private:
        EntityID mEntityA;
        EntityID mEntityB;

        float mHooksConstant = 0;
        float mRestLength    = 0;
    };
} // namespace Umbra
