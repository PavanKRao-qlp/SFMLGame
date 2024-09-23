#pragma once
#include "EnginePCH.h"
#include "ECS/ECSConfig.h"

namespace UMBRA
{

    // class Entity
    // {
    // public:
    //     inline Entity(EntityID _id)
    //     {
    //         mId = _id;
    //     }
    //     inline const EntityID GetId() { return mId; }

    //     bool bAlive;

    // private:
    //     EntityID mId;
    // };

    class EntityManager
    {
    public:
        EntityID CreateEntity();
        void DestroyEntity(EntityID _id);
        bool IsValid(EntityID _id);
        EntityID GetEntityCount();
        Set<EntityID> EntitiesAdded;
        Set<EntityID> EntitiesRemoved;
        Set<EntityID> Entities;
    private:
        BitField<MAX_ENTITY> mEntityAliveFlags;
        Queue<EntityID> mFreeIds;
        EntityID mNextEntityID = 0;
        EntityID mAliveCount = 0;
    };
}

#include "EnityManager.inl"