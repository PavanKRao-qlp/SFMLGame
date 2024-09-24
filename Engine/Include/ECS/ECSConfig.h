#pragma once
#include "EnginePCH.h"
namespace Umbra {
    using EntityID          = uint64;
    const uint32 MAX_ENTITY = 0x7fffffff;

    using ComponentID                = uint64;
    const ComponentID MAX_COMPONENTS = 64; // has to be multiple WORD size for bit set optimization ?
    using ComponentMask              = BitField<MAX_COMPONENTS>;
    const int MIN_POOL_SIZE          = 50;

    class ECSRegister;
} // namespace Umbra
