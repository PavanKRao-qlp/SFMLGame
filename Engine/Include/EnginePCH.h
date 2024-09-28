#pragma once
#include <bitset>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <stdio.h>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace Umbra {
#define CAST(X, Y) reinterpret_cast<X>(Y)
    template <size_t T>
    using BitField = std::bitset<T>;

    using int8  = int8_t;
    using int16 = int16_t;
    using int32 = int32_t;
    using int64 = int64_t;

    using uint8  = uint8_t;
    using uint16 = uint16_t;
    using uint32 = uint32_t;
    using uint64 = uint64_t;
    using String = std::string;

    using Timestamp = uint64_t;
    // template <typename T>
    // using MakeShared = std::make_shared<T>;
    template <typename T>
    using UniquePtr = std::unique_ptr<T>;
    template <typename T>
    using SharedPtr = std::shared_ptr<T>;
    template <typename T>
    using WeakPtr = std::weak_ptr<T>;

    template <typename T>
    using Vector = std::vector<T>; // dynamic Array

    template <typename T, size_t Size>
    using Array = std::array<T, Size>; // dynamic Array

    template <typename T>
    using Queue = std::queue<T>; // dynamic Array

    template <typename T>
    using Stack = std::stack<T>; // dynamic Array

    template <typename Key, typename Val>
    using Map = std::map<Key, Val>;

    template <typename Key, typename Val>
    using UMap = std::unordered_map<Key, Val>;

    template <typename Val>
    using Set = std::set<Val>;
} // namespace Umbra
