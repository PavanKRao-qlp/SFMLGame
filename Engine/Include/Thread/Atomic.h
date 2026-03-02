#pragma once
#include "EnginePCH.h"
#include <atomic>
#include <type_traits>

namespace Umbra {

    // Engine-side alias over std::memory_order — see Atomic<T> comments for meaning
    enum class EMemoryOrder : int {
        Relaxed = static_cast<int>(std::memory_order_relaxed), // no ordering, just atomicity
        Acquire = static_cast<int>(std::memory_order_acquire), // no reads/writes move before this load
        Release = static_cast<int>(std::memory_order_release), // no reads/writes move after this store
        AcqRel  = static_cast<int>(std::memory_order_acq_rel), // both acquire + release (read-modify-write)
        SeqCst  = static_cast<int>(std::memory_order_seq_cst), // full ordering, single global sequence
    };

    // -------------------------------------------------------------------------
    // Atomic<T> — thin wrapper over std::atomic<T>
    //
    // Provides engine naming conventions and a typed EMemoryOrder enum instead
    // of the raw std::memory_order. All methods forward directly to std::atomic
    // with zero overhead (everything is inline).
    //
    // Arithmetic methods (FetchAdd, ++, +=, etc.) are only enabled for
    // integral types via SFINAE — trying them on float/struct won't compile.
    //
    // Convenience aliases at the bottom (AtomicInt32, AtomicBool, etc.)
    // -------------------------------------------------------------------------
    template <typename T>
    class Atomic {
    public:
        Atomic() = default;
        explicit Atomic(T _value) : mValue(_value) {}

        Atomic(const Atomic&)            = delete;
        Atomic& operator=(const Atomic&) = delete;

        // ----- Core operations -----------------------------------------------

        // Write a value. Release or SeqCst is typical for producers.
        void Store(T _value, EMemoryOrder _order = EMemoryOrder::SeqCst) {
            mValue.store(_value, ToStd(_order));
        }

        // Read a value. Acquire or SeqCst is typical for consumers.
        T Load(EMemoryOrder _order = EMemoryOrder::SeqCst) const {
            return mValue.load(ToStd(_order));
        }

        // Atomically write _desired, return the previous value.
        T Exchange(T _desired, EMemoryOrder _order = EMemoryOrder::SeqCst) {
            return mValue.exchange(_desired, ToStd(_order));
        }

        // If current value == _expected: write _desired, return true.
        // If not: write current value into _expected, return false.
        // Strong: no spurious failures. Use in regular if/else logic.
        bool CompareExchangeStrong(T& _expected, T _desired,
            EMemoryOrder _success = EMemoryOrder::SeqCst,
            EMemoryOrder _failure = EMemoryOrder::SeqCst) {
            return mValue.compare_exchange_strong(_expected, _desired, ToStd(_success), ToStd(_failure));
        }

        // Same as Strong but may fail spuriously. Use in a retry loop —
        // compiles to a cheaper instruction on ARM (LL/SC based architectures).
        bool CompareExchangeWeak(T& _expected, T _desired,
            EMemoryOrder _success = EMemoryOrder::SeqCst,
            EMemoryOrder _failure = EMemoryOrder::SeqCst) {
            return mValue.compare_exchange_weak(_expected, _desired, ToStd(_success), ToStd(_failure));
        }

        // True if this atomic doesn't internally use a mutex (always prefer lock-free)
        bool IsLockFree() const noexcept { return mValue.is_lock_free(); }

        // ----- Convenience operators -----------------------------------------

        // Implicit read — lets you write: T val = myAtomic;
        operator T() const { return Load(); }

        // Assignment — lets you write: myAtomic = value;
        T operator=(T _value) {
            Store(_value);
            return _value;
        }

        // ----- Integral-only arithmetic (SFINAE-gated) -----------------------
        // These won't exist on Atomic<float>, Atomic<MyStruct>, etc.

        template <typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
        T FetchAdd(T _delta, EMemoryOrder _order = EMemoryOrder::SeqCst) {
            return mValue.fetch_add(_delta, ToStd(_order));
        }

        template <typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
        T FetchSub(T _delta, EMemoryOrder _order = EMemoryOrder::SeqCst) {
            return mValue.fetch_sub(_delta, ToStd(_order));
        }

        template <typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
        T FetchAnd(T _mask, EMemoryOrder _order = EMemoryOrder::SeqCst) {
            return mValue.fetch_and(_mask, ToStd(_order));
        }

        template <typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
        T FetchOr(T _mask, EMemoryOrder _order = EMemoryOrder::SeqCst) {
            return mValue.fetch_or(_mask, ToStd(_order));
        }

        template <typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
        T FetchXor(T _mask, EMemoryOrder _order = EMemoryOrder::SeqCst) {
            return mValue.fetch_xor(_mask, ToStd(_order));
        }

        // Pre-increment: ++a  — returns new value
        template <typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
        T operator++() { return FetchAdd(1) + 1; }

        // Post-increment: a++  — returns old value
        template <typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
        T operator++(int) { return FetchAdd(1); }

        // Pre-decrement: --a  — returns new value
        template <typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
        T operator--() { return FetchSub(1) - 1; }

        // Post-decrement: a--  — returns old value
        template <typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
        T operator--(int) { return FetchSub(1); }

        template <typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
        T operator+=(T _delta) { return FetchAdd(_delta) + _delta; }

        template <typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
        T operator-=(T _delta) { return FetchSub(_delta) - _delta; }

        // Direct access for advanced use (e.g. passing to std APIs)
        std::atomic<T>&       GetNative() { return mValue; }
        const std::atomic<T>& GetNative() const { return mValue; }

    private:
        static constexpr std::memory_order ToStd(EMemoryOrder _order) {
            return static_cast<std::memory_order>(static_cast<int>(_order));
        }

        std::atomic<T> mValue{};
    };

    // -------------------------------------------------------------------------
    // Convenience aliases
    // -------------------------------------------------------------------------
    using AtomicBool   = Atomic<bool>;
    using AtomicInt8   = Atomic<int8>;
    using AtomicInt16  = Atomic<int16>;
    using AtomicInt32  = Atomic<int32>;
    using AtomicInt64  = Atomic<int64>;
    using AtomicUInt8  = Atomic<uint8>;
    using AtomicUInt16 = Atomic<uint16>;
    using AtomicUInt32 = Atomic<uint32>;
    using AtomicUInt64 = Atomic<uint64>;

} // namespace Umbra
