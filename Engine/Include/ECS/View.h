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
        ComponentArrayPool *ComponentArrayHolder;
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
        template <typename ComponentType1, typename... ComponentTypeRest>
        void BuildComponentArray();

        template <typename ComponentType1, typename... ComponentTypeRest>
        void AddToComponentArray(EntityID _entity);

        template <typename ComponentType1, typename... ComponentTypeRest>
        void RemoveFromComponentArray(EntityID _entity);

    private:
    };

    template <typename... ComponentTypes>
    ECView<ComponentTypes...>::ECView()
    {
        mEntities.clear();
        ComponentArrayHolder = new ComponentArrayPool();
        mSignature = CreateSignature<ComponentTypes...>();
        BuildComponentArray<ComponentTypes...>();
    }

    template <typename... ComponentTypes>
    void ECView<ComponentTypes...>::AddEntity(EntityID _entity)
    {
        if (mEntities.find(_entity) == mEntities.end())
        {
            mEntities.emplace(_entity);
            AddToComponentArray<ComponentTypes...>(_entity);
        }
    }

    template <typename... ComponentTypes>
    void ECView<ComponentTypes...>::RemoveEntity(EntityID _entity)
    {
        RemoveFromComponentArray<ComponentTypes...>(_entity);
    }

    template <typename... ComponentTypes>
    template <typename ComponentType1, typename... ComponentTypeRest>
    void ECView<ComponentTypes...>::BuildComponentArray()
    {
        ComponentArrayHolder->AddComponentArray(new ComponentArray<ComponentType1>());
        if constexpr (sizeof...(ComponentTypeRest) > 0)
        {
            BuildComponentArray<ComponentTypeRest...>();
        }
    }

    template <typename... ComponentTypes>
    template <typename ComponentType1, typename... ComponentTypeRest>
    void ECView<ComponentTypes...>::AddToComponentArray(EntityID _entity)
    {

        ComponentArray<ComponentType1> *componentArray = ComponentArrayHolder->GetComponentArray<ComponentType1>();
        if (!componentArray->Has(_entity))
        {
            ComponentType1 *component = ecsRegister->GetComponent<ComponentType1>(_entity);
            componentArray->Insert(_entity, *component);
        }
        if constexpr (sizeof...(ComponentTypeRest) > 0)
        {
            AddToComponentArray<ComponentTypeRest...>(_entity);
        }
    }

    template <typename... ComponentTypes>
    template <typename ComponentType1, typename... ComponentTypeRest>
    void ECView<ComponentTypes...>::RemoveFromComponentArray(EntityID _entity)
    {
        ComponentArray<ComponentType1> *componentArray = ComponentArrayHolder->GetComponentArray<ComponentType1>();
        if (componentArray->Has(_entity))
        {
            ComponentType1 *component = ecsRegister->GetComponent<ComponentType1>(_entity);
            componentArray->Remove(_entity, *component);
        }
        if constexpr (sizeof...(ComponentTypeRest) > 0)
        {
            RemoveFromComponentArray<ComponentTypeRest...>(_entity);
        }
    }
}