#pragma once
#include "EnginePCH.h"
#include "ECS/ECSConfig.h"
#include "ECS/Enity.h"
#include "ECS/Component.h"
#include "ECS/ECSRegister.h"

namespace D2D
{
    class BaseView
    {
    public:
        inline ComponentMask GetSignature() { return mSignature; };
        inline void AssignRegistry(ECSRegister *_register) { ecsRegister = _register; };
        virtual void AddEntity(EntityID _entity) = 0;
        virtual void RemoveEntity(EntityID _entity) = 0;

        // protected:
        Set<EntityID> mEntities;
        ComponentMask mSignature;
        ECSRegister *ecsRegister;
        EntityID id;
    };

    template <typename... ComponentTypes>
    class ECView : public BaseView
    {
    public:
        ECView();
        virtual void AddEntity(EntityID _entity) override;
        virtual void RemoveEntity(EntityID _entity) override;

    private:
    };

    template <typename... ComponentTypes>
    ECView<ComponentTypes...>::ECView()
    {
        mEntities.clear();
        mSignature = CreateSignature<ComponentTypes...>();
    }

    template <typename... ComponentTypes>
    void ECView<ComponentTypes...>::AddEntity(EntityID _entity)
    {
        if (mEntities.find(_entity) == mEntities.end())
        {
            mEntities.emplace(_entity);
        }
    }

    template <typename... ComponentTypes>
    void ECView<ComponentTypes...>::RemoveEntity(EntityID _entity)
    {
        if (mEntities.find(_entity) != mEntities.end())
        {
            mEntities.erase(_entity);
        }
    }
}