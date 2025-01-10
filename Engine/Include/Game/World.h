#pragma once
#include "ECS/ECSRegister.h"
#include "Umbra.h"

namespace Umbra {
    class World {
    public:
        void SetECSRegister(ECSRegister* _worldRegister);
        ECSRegister* GetRegister();

    private:
        ECSRegister* mWorldRegister;
    };

} // namespace Umbra
