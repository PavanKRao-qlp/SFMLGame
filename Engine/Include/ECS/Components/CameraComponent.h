#pragma once
#include "ECS/Component.h"
namespace Umbra {
    struct CameraComponent : Component {
    public:
        inline CameraComponent() {}
        inline CameraComponent(const float _orthographicSize) : mOrthographicSize(_orthographicSize) {}
        inline bool IsActive() {
            return bActive;
        }
        inline void SetActive(bool _bActive) {
            bActive = _bActive;
        }
        inline const float GetOrthographicSize() {
            return mOrthographicSize;
        }
        inline const void SetOrthographicSize(float _size) {
            mOrthographicSize = _size;
        }

    protected:
        float mOrthographicSize = 0;
        bool bActive            = false;
    };
} // namespace Umbra
