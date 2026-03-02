#pragma once
#include "EnginePCH.h"

namespace Umbra {

    /// @brief Opaque handle to a physics body in the PhysicsService
    /// Uses generational index pattern to detect stale handles
    struct BodyHandle {
        uint32 Index      = UINT32_MAX;
        uint32 Generation = 0;

        inline bool IsValid() const {
            return Index != UINT32_MAX;
        }

        inline bool operator==(const BodyHandle& _other) const {
            return Index == _other.Index && Generation == _other.Generation;
        }

        inline bool operator!=(const BodyHandle& _other) const {
            return !(*this == _other);
        }

        static BodyHandle Invalid() {
            return BodyHandle{UINT32_MAX, 0};
        }
    };

    /// @brief Opaque handle to a physics constraint in the PhysicsService
    /// Uses generational index pattern to detect stale handles
    struct ConstraintHandle {
        uint32 Index      = UINT32_MAX;
        uint32 Generation = 0;

        inline bool IsValid() const {
            return Index != UINT32_MAX;
        }

        inline bool operator==(const ConstraintHandle& _other) const {
            return Index == _other.Index && Generation == _other.Generation;
        }

        inline bool operator!=(const ConstraintHandle& _other) const {
            return !(*this == _other);
        }

        static ConstraintHandle Invalid() {
            return ConstraintHandle{UINT32_MAX, 0};
        }
    };

} // namespace Umbra
