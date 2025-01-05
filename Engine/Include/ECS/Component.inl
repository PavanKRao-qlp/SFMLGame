#include "Component.h"

namespace Umbra {

    template <typename T>
    inline ComponentArray<T>::ComponentArray() {
        mPackedComponents.reserve(MIN_POOL_SIZE);
    }

    template <typename T>
    inline T& ComponentArray<T>::Get(EntityID _entity) {
        int ix = mSparseIndexMap.at(_entity);
        return mPackedComponents[ix];
    }

    template <typename T>
    inline void ComponentArray<T>::Insert(EntityID _entity, T _component) {
        mPackedComponents.emplace_back(_component);
        mSparseIndexMap[_entity]                       = (int) mPackedComponents.size() - 1;
        mDenseToSparseKey[mSparseIndexMap.at(_entity)] = _entity;
    }

    template <typename T>
    inline bool ComponentArray<T>::Has(EntityID _entity) {
        return mSparseIndexMap.find(_entity) != mSparseIndexMap.end();
    }

    template <typename T>
    inline bool ComponentArray<T>::Remove(EntityID _entity) {
        if (mSparseIndexMap.find(_entity) != mSparseIndexMap.end()) {
            Remove(_entity, Get(_entity));
            return true;
        }
        return false;
    }

    template <typename T>
    inline void ComponentArray<T>::Remove(EntityID _entity, T _component) {
        int packedIx     = mSparseIndexMap[_entity];
        int lastPackedIx = (int) mPackedComponents.size() - 1;

        if (packedIx != lastPackedIx) {
            mPackedComponents[packedIx] = mPackedComponents.at(lastPackedIx);
            EntityID swapEntity         = mDenseToSparseKey.at(lastPackedIx);
            mSparseIndexMap[swapEntity] = packedIx;
            mDenseToSparseKey[packedIx] = swapEntity;
        }
        mDenseToSparseKey.erase(lastPackedIx);
        mPackedComponents.pop_back();
        mSparseIndexMap.erase(_entity);
    }
}; // namespace Umbra
