#pragma once
#include <stdio.h>
#include <string>
#include <functional>
#include <map>
#include <typeindex>
#include <typeinfo>

namespace D2D
{
    #define CAST(X,Y) reinterpret_cast<X>(Y)

    using int8 = int8_t;
    using int16 = int16_t;
    using int32 = int32_t;
    using int64 = int64_t;

    using uint8 = uint8_t;
    using uint16 = uint16_t;
    using uint32 = uint32_t;
    using uint64 = uint64_t;

}