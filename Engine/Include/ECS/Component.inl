#include "Component.h"

namespace Umbra {

    template <typename T>
    inline ComponentArray<T>::ComponentArray() {
        mPackedComponents.reserve(MIN_POOL_SIZE);
    }

    template <typename T>
    inline T& ComponentArray<T>::Get(EntityID _entity) {
        int ix = mSparseIndexMap[_entity];
        return mPackedComponents[ix];
    }

    template <typename T>
    inline void ComponentArray<T>::Insert(EntityID _entity, T _component) {
        mPackedComponents.emplace_back(_component);
        mSparseIndexMap[_entity] = (int) mPackedComponents.size() - 1;
    }
    template <typename T>
    inline bool ComponentArray<T>::Has(EntityID _entity) {
        return mSparseIndexMap.find(_entity) != mSparseIndexMap.end();
    }

    template <typename T>
    inline void ComponentArray<T>::Remove(EntityID _entity, T _component) {
        // mPackedComponents(_component);
        // mSparseIndexMap[_entity] = (int)mPackedComponents.size() - 1;
    }
}; // namespace Umbra
