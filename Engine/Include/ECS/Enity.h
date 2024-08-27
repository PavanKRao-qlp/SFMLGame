#pragma once
#include "EnginePCH.h"
#include "ECS/ECSConfig.h"

namespace D2D
{

    class Entity
    {
    public:
        inline Entity(EntityID _id)
        {
            mId = _id;
        }
        inline const EntityID GetId() { return mId; }

        bool bAlive;

    private:
        EntityID mId;
    };
}