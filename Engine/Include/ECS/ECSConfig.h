#pragma once
#include "EnginePCH.h"
namespace UMBRA
{
    using EntityID = uint64;
    const uint64 MAX_ENTITY = 10;

    using ComponentID = uint64;
    const ComponentID MAX_COMPONENTS = 64; // has to be multiple WORD size for bit set optimization ?
    using ComponentMask = BitField<MAX_COMPONENTS>;
    const int MIN_POOL_SIZE = 50;

    class ECSRegister;
}