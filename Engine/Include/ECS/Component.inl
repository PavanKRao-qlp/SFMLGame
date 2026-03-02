#include "Component.h"

namespace Umbra {

    template <typename T>
    inline ComponentArray<T>::ComponentArray() {
        mDense.reserve(MIN_POOL_SIZE);
        mDenseEntities.reserve(MIN_POOL_SIZE);
    }

    template <typename T>
    inline ComponentArray<T>::~ComponentArray() {
    }

    template <typename T>
    inline T& ComponentArray<T>::Get(EntityID _entity) {
        size_t denseIndex = mSparse[_entity];
        return mDense[denseIndex];
    }

    template <typename T>
    inline void ComponentArray<T>::Insert(EntityID _entity, T _component) {
        // Grow sparse array if needed
        if (_entity >= mSparse.size()) {
            mSparse.resize(_entity + 1, INVALID_INDEX);
        }

        size_t denseIndex    = mDense.size();
        mSparse[_entity]     = denseIndex;
        mDense.emplace_back(std::move(_component));
        mDenseEntities.emplace_back(_entity);
    }

    template <typename T>
    inline bool ComponentArray<T>::Has(EntityID _entity) const {
        return _entity < mSparse.size() && mSparse[_entity] != INVALID_INDEX;
    }

    template <typename T>
    inline bool ComponentArray<T>::Remove(EntityID _entity) {
        if (!Has(_entity)) {
            return false;
        }

        size_t removedIndex = mSparse[_entity];
        size_t lastIndex    = mDense.size() - 1;

        if (removedIndex != lastIndex) {
            // Swap with last element
            mDense[removedIndex]         = std::move(mDense[lastIndex]);
            mDenseEntities[removedIndex] = mDenseEntities[lastIndex];

            // Update sparse index for swapped entity
            EntityID swappedEntity   = mDenseEntities[removedIndex];
            mSparse[swappedEntity]   = removedIndex;
        }

        // Remove last element
        mDense.pop_back();
        mDenseEntities.pop_back();
        mSparse[_entity] = INVALID_INDEX;

        return true;
    }

    template <typename T>
    inline void ComponentArray<T>::Flush() {
        mDense.clear();
        mDenseEntities.clear();
        mSparse.clear();
    }

}; // namespace Umbra
