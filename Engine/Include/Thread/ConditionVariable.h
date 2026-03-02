#pragma once
#include "EnginePCH.h"
#include "Mutex.h"
#include <chrono>
#include <condition_variable>

namespace Umbra {

    // -------------------------------------------------------------------------
    // UniqueLock — RAII scoped lock that supports manual unlock/relock.
    //
    // Required by ConditionVariable::Wait() because the CV needs to
    // release the lock while sleeping, then reacquire it on wakeup.
    // A regular LockGuard cannot do this (it only unlocks on destruction).
    //
    //   Mutex mtx;
    //   UniqueLock lock(mtx);       // locks immediately
    //   lock.Unlock();              // manually release
    //   lock.Lock();                // manually reacquire
    //    auto-unlocks on scope exit
    //
    // -------------------------------------------------------------------------
    class UniqueLock {
    public:
        explicit UniqueLock(Mutex& _mutex) : mMutex(_mutex), mOwns(true) {
            mMutex.Lock();
        }

        ~UniqueLock() {
            if (mOwns) {
                mMutex.Unlock();
            }
        }

        UniqueLock(const UniqueLock&)            = delete;
        UniqueLock& operator=(const UniqueLock&) = delete;

        void Lock() {
            mMutex.Lock();
            mOwns = true;
        }

        void Unlock() {
            mMutex.Unlock();
            mOwns = false;
        }

        bool OwnsLock() const {
            return mOwns;
        }

        // Expose the underlying std::unique_lock to the CV internals.
        // The standard CV only accepts std::unique_lock<std::mutex>.
        std::unique_lock<std::mutex> StdLock() {
            // adopt_lock: std::unique_lock takes ownership of an already-locked mutex
            // We hand it our mutex; the CV will release/reacquire it during wait.
            return std::unique_lock<std::mutex>(mMutex.GetStd(), std::adopt_lock);
        }

        Mutex& GetMutex() {
            return mMutex;
        }

    private:
        Mutex& mMutex;
        bool mOwns;
    };

    // -------------------------------------------------------------------------
    // ConditionVariable — lets threads sleep until a condition becomes true.
    //
    // HOW IT WORKS:
    //   Wait() atomically: releases the mutex, puts the thread to sleep.
    //   When another thread calls Notify() or NotifyAll(), the sleeping
    //   thread wakes up, reacquires the mutex, then rechecks the predicate.
    //
    //   Spurious wakeups: the OS can wake a thread for no reason. Always
    //   pass a predicate to Wait() — it loops internally until true.
    //
    // TYPICAL PATTERN:
    //
    //   // Producer
    //   {
    //       LockGuard<Mutex> lock(mtx);
    //       data = produce();
    //       bReady = true;
    //   }
    //   cv.NotifyOne();
    //
    //   // Consumer
    //   {
    //       UniqueLock lock(mtx);
    //       cv.Wait(lock, [&]{ return bReady; });
    //       consume(data);
    //   }
    //
    // RULES:
    //   - Always hold the mutex when calling Wait()
    //   - Always hold the mutex when modifying the condition
    //   - Notify can be called with or without the mutex held
    //     (holding it avoids a rare missed-wakeup in edge cases)
    // -------------------------------------------------------------------------
    class ConditionVariable {
    public:
        ConditionVariable()  = default;
        ~ConditionVariable() = default;

        ConditionVariable(const ConditionVariable&)            = delete;
        ConditionVariable& operator=(const ConditionVariable&) = delete;
        ConditionVariable(ConditionVariable&&)                 = delete;
        ConditionVariable& operator=(ConditionVariable&&)      = delete;

        // ── Wait (with predicate) ─────────────────────────────────────────────
        // Sleeps until _predicate() returns true.
        // Handles spurious wakeups automatically.
        // _lock must already be locked.
        template <typename Predicate>
        void Wait(UniqueLock& _lock, Predicate _predicate) {
            // Hand a std::unique_lock (adopt_lock) to the CV.
            // When wait() returns, std::unique_lock releases ownership
            // so we don't double-unlock — we give mOwns back manually.
            auto stdLock = _lock.StdLock();
            mCV.wait(stdLock, std::move(_predicate));
            // stdLock now holds the mutex again; release without unlocking
            // so UniqueLock remains the true owner
            stdLock.release();
        }

        // ── Wait (no predicate) ───────────────────────────────────────────────
        // Raw wait — susceptible to spurious wakeups.
        // Prefer the predicate overload unless you handle spurious wakeups yourself.
        void Wait(UniqueLock& _lock) {
            auto stdLock = _lock.StdLock();
            mCV.wait(stdLock);
            stdLock.release();
        }

        // ── WaitFor ───────────────────────────────────────────────────────────
        // Waits at most _ms milliseconds, or until the predicate is true.
        // Returns true if the predicate was satisfied, false on timeout.
        template <typename Predicate>
        bool WaitFor(UniqueLock& _lock, uint32 _ms, Predicate _predicate) {
            auto stdLock = _lock.StdLock();
            bool result  = mCV.wait_for(stdLock, std::chrono::milliseconds(_ms), std::move(_predicate));
            stdLock.release();
            return result;
        }

        // ── Notify ────────────────────────────────────────────────────────────

        // Wake one waiting thread
        void NotifyOne() {
            mCV.notify_one();
        }

        // Wake all waiting threads
        void NotifyAll() {
            mCV.notify_all();
        }

    private:
        std::condition_variable mCV;
    };

} // namespace Umbra
