#pragma once
#include "EnginePCH.h"

namespace D2D
{

    template <typename T, std::size_t Capacity>
    class SparseArray
    {
    public:
        void Insert(std::size_t _index, const T &_value);
        void RemoveAt();

    private:
        Vector<T> Dense;
        //  Queue
        // BitField<Capacity> SparseIndices;
    };

    template <typename T, std::size_t Capacity>
    void SparseArray<T, Capacity>::Insert(std::size_t _index, const T &_value) {
        
    };
}