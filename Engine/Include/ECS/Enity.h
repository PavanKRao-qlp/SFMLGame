#pragma once
#include "ECS/ECSConfig.h"
#include "EnginePCH.h"

namespace Umbra {
    class EntityManager {
    public:
        EntityID CreateEntity();
        void DestroyEntity(EntityID _id);
        void RemoveEntity(EntityID _id);
        bool IsValid(EntityID _id);
        void Flush();
        EntityID GetEntityCount();
        Set<EntityID> EntitiesAdded;
        Set<EntityID> EntitiesDestroyed;
        Set<EntityID> EntitiesModified;
        Set<EntityID> Entities;

    private:
        BitField<MAX_ENTITY> mEntityAliveFlags;
        Queue<EntityID> mFreeIds;
        EntityID mNextEntityID = 0;
        EntityID mAliveCount   = 0;
    };
} // namespace Umbra

#include "EnityManager.inl"
