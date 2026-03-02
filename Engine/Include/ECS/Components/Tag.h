#pragma once
#include "ECS/Component.h"
#include "ECS/Enity.h"
#include "Math/Vector.h"
namespace Umbra {
    struct TagComponent : Component {
    public:
        inline TagComponent(String _Tag) : Tag(_Tag) {}
        String Tag;
    };
} // namespace Umbra
