#include "Enity.h"

namespace UMBRA
{
    inline EntityID EntityManager::CreateEntity()
    {
        EntityID entityID = MAX_ENTITY;
        // Check if Free indexes exist
        if (mFreeIds.size() > 0)
        {
            entityID = mFreeIds.front();
            mFreeIds.pop();
        }
        else
        {
            if (mNextEntityID >= MAX_ENTITY)
            {
                // ASSERT (MAX ENTITY REACHED!)
                return entityID;
            }
            else
            {
                entityID = mNextEntityID++;
            }
        }
        //@todo smart Pointer
        mEntityAliveFlags.set(entityID, true);
        EntitiesAdded.emplace(entityID);
        mAliveCount++;
        return entityID;
    };

    inline void EntityManager::DestroyEntity(EntityID _id)
    {
        EntitiesRemoved.emplace(_id);
        mEntityAliveFlags.set(_id, false);
        mFreeIds.push(_id);
        mAliveCount--;
    };

    inline bool EntityManager::IsValid(EntityID _id)
    {
        return  _id < MAX_ENTITY && mEntityAliveFlags.test(_id);
    }

    inline EntityID EntityManager::GetEntityCount()
    {
        return mAliveCount;
    }
}