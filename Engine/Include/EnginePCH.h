#pragma once
#include <stdio.h>
#include <string>
#include <functional>

#include <map>
#include <vector>
#include <queue>
#include <deque>
#include <stack>
#include <set>
#include <unordered_map>

#include <typeindex>
#include <typeinfo>

#include <bitset>

namespace D2D
{
    #define CAST(X,Y) reinterpret_cast<X>(Y)
    template<size_t T>
    using BitField = std::bitset<T>;
    
    using int8 = int8_t;
    using int16 = int16_t;
    using int32 = int32_t;
    using int64 = int64_t;

    using uint8 = uint8_t;
    using uint16 = uint16_t;
    using uint32 = uint32_t;
    using uint64 = uint64_t;
    using String = std::string;

    using Timestamp = uint64_t;
    
    template<typename T>
    using Vector = std::vector<T>; //dynamic Array
    
    template<typename T, size_t Size>
    using Array = std::array<T,Size>; //dynamic Array

    template<typename T>
    using Queue = std::queue<T>; //dynamic Array

    template<typename T>
    using Stack = std::stack<T>; //dynamic Array
    
    template<typename Key, typename Val>
    using Map = std::map<Key,Val>;
    
    template<typename Key, typename Val>
    using UMap = std::unordered_map<Key,Val>;
    
    template<typename Val>
    using Set = std::set<Val>;
}