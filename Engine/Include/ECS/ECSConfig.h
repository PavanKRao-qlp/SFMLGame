#pragma once
#include "EnginePCH.h"
namespace D2D
{
    using EntityID = int64;
    const int MAX_ENTITY = 1000;

    using ComponentID = uint64;
    const ComponentID MAX_COMPONENTS = 64; // has to be multiple WORD size for bit set optimization ?
    using ComponentMask = BitField<MAX_COMPONENTS>;
    const int MIN_POOL_SIZE = 50;

    class ECSRegister;
}