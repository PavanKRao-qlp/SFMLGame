// #pragma once
// #include "Core/Singleton.h"
// #include "ECS/ECSRegister.h"
// #include "EnginePCH.h"
// #include "Math/MathUtils.h"

// namespace Umbra2 {

//     class CollisionDetectionSystem;
//     class CollisionEventResolverSystem;
//     namespace Collision {
//         class CollisionSystem;
//         const uint8 MAX_COLLISION_CHANNEL = 16;
//         using CollisionMask               = BitField<MAX_COLLISION_CHANNEL>;

//         enum class ECollisionChannel : unsigned int {
//             IGNORE   = 0,
//             STATIC   = 1,
//             Option2  = 2,
//             Option3  = 3,
//             Option4  = 4,
//             Option5  = 5,
//             Option6  = 6,
//             Option7  = 7,
//             Option8  = 8,
//             Option9  = 9,
//             Option10 = 10,
//             Option11 = 11,
//             Option12 = 12,
//             Option13 = 13,
//             Option14 = 14,
//             Option15 = 15
//         };

//         struct CollisionResponse {
//         public:
//             EntityID mEntityA;
//             EntityID mEntityB;
//         };

//     } // namespace Collision
// } // namespace Umbra2
