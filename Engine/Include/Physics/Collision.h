#pragma once
#include "ECS/Components/Collider.h"
#include "ECS/Components/PhysicsBodyComponent.h"
#include "ECS/Components/Transform.h"
#include "ECS/Enity.h"
#include "Math/Polygon.h"
#include "Umbra.h"

namespace Umbra {


    class ContactPoint {
    public:
        Math::Vector2f mContactNormal;
        Math::Vector2f mContactPosition;
        double mPenetration;
    };

    class Collision {
    public:
        EntityID mBodyA;
        EntityID mBodyB;
        double mCofOfRestitution;
        Math::Vector2f mContactNormal;
        void AddContact(ContactPoint& _contact);
        void AddContacts(Vector<ContactPoint>& _contacts);
        Vector<ContactPoint>& GetContacts();

    private:
        Vector<ContactPoint> mContacts;
    };

} // namespace Umbra
