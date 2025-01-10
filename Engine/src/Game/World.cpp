#include "Game/World.h"

namespace Umbra {

    void World::SetECSRegister(ECSRegister* _worldRegister) {
        mWorldRegister = _worldRegister;
    }

    ECSRegister* World::GetRegister() {
        return mWorldRegister;
    }

} // namespace Umbra
