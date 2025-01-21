#include "Enity.h"

namespace Umbra {
    inline EntityID EntityManager::CreateEntity() {
        EntityID entityID = MAX_ENTITY;
        // Check if Free indexes exist
        if (mFreeIds.size() > 0) {
            entityID = mFreeIds.front();
            mFreeIds.pop();
        } else {
            if (mNextEntityID >= MAX_ENTITY) {
                // ASSERT (MAX ENTITY REACHED!)
                return entityID;
            } else {
                entityID = mNextEntityID++;
            }
        }
        //@todo smart Pointer
        mEntityAliveFlags.set(entityID, true);
        EntitiesAdded.emplace(entityID);
        mAliveCount++;
        return entityID;
    }

    inline void EntityManager::DestroyEntity(EntityID _id) {
        if (EntitiesDestroyed.find(_id) == EntitiesDestroyed.end()) {
            EntitiesDestroyed.emplace(_id);
        }
    }

    inline void EntityManager::RemoveEntity(EntityID _id) {
        Entities.erase(_id);
        mEntityAliveFlags.set(_id, false);
        mFreeIds.push(_id);
        mAliveCount--;
    }

    inline bool EntityManager::IsValid(EntityID _id) {
        return _id < MAX_ENTITY && mEntityAliveFlags.test(_id);
    }

    inline void EntityManager::Flush() {
        Entities.clear();
        EntitiesAdded.clear();
        EntitiesDestroyed.clear();
        mEntityAliveFlags.reset();
        while (mFreeIds.size() > 0) {
            mFreeIds.pop();
        };
        mNextEntityID = 0;
        mAliveCount   = 0;
    }

    inline EntityID EntityManager::GetEntityCount() {
        return mAliveCount;
    }
} // namespace Umbra
