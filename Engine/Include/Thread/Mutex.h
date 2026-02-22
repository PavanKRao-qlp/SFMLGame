#pragma once
#include "EnginePCH.h"
#include "Thread/Atomic.h"
#include <mutex>

// CPU pause hint — reduces power and improves throughput in spin loops
// by telling the CPU we are intentionally busy-waiting
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#include <immintrin.h>
#define UMBRA_CPU_PAUSE() _mm_pause()
#elif defined(__ARM_ARCH)
#define UMBRA_CPU_PAUSE() __asm__ volatile("yield")
#else
#define UMBRA_CPU_PAUSE() ((void) 0)
#endif

namespace Umbra {

    // -------------------------------------------------------------------------
    // LockGuard — RAII scope lock, works with both Mutex and SpinLock
    //
    //   Mutex mtx;
    //   {
    //       LockGuard<Mutex> lock(mtx);  // locks here
    //       // ... critical section ...
    //   }                                // unlocks here automatically
    // -------------------------------------------------------------------------
    template <typename TMutex>
    class LockGuard {
    public:
        explicit LockGuard(TMutex& _mutex) : mMutex(_mutex) {
            mMutex.Lock();
        }
        ~LockGuard() {
            mMutex.Unlock();
        }

        LockGuard(const LockGuard&)            = delete;
        LockGuard& operator=(const LockGuard&) = delete;

    private:
        TMutex& mMutex;
    };

    // -------------------------------------------------------------------------
    // Mutex — wraps std::mutex (OS-level blocking primitive)
    //
    // HOW IT WORKS:
    //   When a thread calls Lock() and the mutex is already held, the OS
    //   puts that thread to sleep and schedules something else. When the
    //   owner calls Unlock(), the OS wakes one waiting thread. This involves
    //   a syscall (kernel/user boundary crossing) which costs ~100-1000ns.
    //
    // USE WHEN:
    //   - The critical section takes more than ~1 microsecond
    //   - Many threads may contend at the same time
    //   - You can't afford to burn CPU cycles waiting
    // -------------------------------------------------------------------------
    class Mutex {
    public:
        Mutex()  = default;
        ~Mutex() = default;

        Mutex(const Mutex&)            = delete;
        Mutex& operator=(const Mutex&) = delete;
        Mutex(Mutex&&)                 = delete;
        Mutex& operator=(Mutex&&)      = delete;

        void Lock() {
            mMutex.lock();
        }
        void Unlock() {
            mMutex.unlock();
        }

        // Returns true if the lock was acquired, false if already held
        bool TryLock() {
            return mMutex.try_lock();
        }

        // Raw access — only needed by ConditionVariable internals
        std::mutex& GetStd() {
            return mMutex;
        }

    private:
        std::mutex mMutex;
    };

    // -------------------------------------------------------------------------
    // SpinLock — user-space busy-wait lock (no OS involvement)
    //
    // HOW IT WORKS:
    //   Uses a single atomic_flag. Lock() loops calling test_and_set() until
    //   it wins (returns false = flag was clear). Unlock() clears the flag.
    //   The looping thread never sleeps — it burns CPU cycles checking.
    //
    //   test_and_set with acquire:  reads the flag AND sets it atomically.
    //                               No other thread can interleave.
    //   clear with release:         ensures all writes inside the critical
    //                               section are visible before the flag drops.
    //
    // USE WHEN:
    //   - The critical section is very short (<1 microsecond, e.g. a pointer swap)
    //   - Contention is rare / few threads
    //   - You're on a multi-core machine (single-core spinning is pure waste)
    //
    // AVOID WHEN:
    //   - The critical section does any I/O, allocation, or heavy computation
    //   - Many threads may contend simultaneously (they all burn a full core)
    // -------------------------------------------------------------------------
    class SpinLock {
    public:
        SpinLock()  = default;
        ~SpinLock() = default;

        SpinLock(const SpinLock&)            = delete;
        SpinLock& operator=(const SpinLock&) = delete;
        SpinLock(SpinLock&&)                 = delete;
        SpinLock& operator=(SpinLock&&)      = delete;

        void Lock() {
            while (mFlag.test_and_set(std::memory_order_acquire)) {
                // Spin — but yield the CPU pipeline to reduce power draw
                // and avoid starving the thread that holds the lock
                UMBRA_CPU_PAUSE();
            }
        }

        void Unlock() {
            // release: all writes above this point become visible to the
            // next thread that acquires the lock
            mFlag.clear(std::memory_order_release);
        }

        // Non-blocking attempt. Returns true if lock was acquired.
        bool TryLock() {
            return !mFlag.test_and_set(std::memory_order_acquire);
        }

    private:
        std::atomic_flag mFlag = ATOMIC_FLAG_INIT;
    };

} // namespace Umbra
