#pragma once
#include "EnginePCH.h"
#include "Thread/Atomic.h"
#include "Thread/Mutex.h"

namespace Umbra {

    // -------------------------------------------------------------------------
    // JobCompletion — reference-counted completion state shared between a
    // JobHandle and the worker thread(s) that execute the associated work.
    //
    // HOW IT WORKS (M2 — Atomic Counters):
    //   mPendingCount starts at 1 for a single job, or N for ParallelFor.
    //   Each worker calls Decrement() when it finishes. When the count
    //   reaches zero, all registered callbacks are fired (used for M3
    //   dependency chaining: the callback submits the dependent job).
    //
    //   acquire-release ordering ensures all writes made inside a job are
    //   visible to any thread that observes mPendingCount == 0.
    //
    // -------------------------------------------------------------------------
    struct JobCompletion {
        AtomicInt32 mPendingCount{1};
        Mutex mCallbackMutex;
        Vector<std::function<void()>> mCallbacks; // fired when count hits 0

        // Called by a worker when the job (or one ParallelFor item) finishes.
        // If this call brings mPendingCount to zero, fires all callbacks.
        void Decrement() {
            // AcqRel: acquire the releases done by all prior workers so their
            // writes are visible here; release so our writes are visible to
            // anyone who reads mPendingCount == 0 with Acquire.
            int32 prev = mPendingCount.FetchSub(1, EMemoryOrder::AcqRel);
            if (prev == 1) {
                // we just set the count to 0
                Vector<std::function<void()>> toFire;
                {
                    LockGuard<Mutex> lock(mCallbackMutex);
                    toFire = std::move(mCallbacks);
                }
                for (auto& cb : toFire) {
                    cb();
                }
            }
        }

        // Register a callback to fire when mPendingCount reaches zero.
        // If already complete, invokes _callback immediately (outside the lock).
        void AddCallback(std::function<void()> _callback) {
            {
                LockGuard<Mutex> lock(mCallbackMutex);
                if (mPendingCount.Load(EMemoryOrder::Acquire) != 0) {
                    mCallbacks.push_back(std::move(_callback));
                    return;
                }
            }
            // Already complete — fire without the lock to avoid deadlock
            _callback();
        }
    };

    // -------------------------------------------------------------------------
    // JobHandle — lightweight value type returned from Submit / SubmitAfter /
    // ParallelFor. Internally holds a shared pointer to completion state.
    //
    // IsComplete()  — non-blocking poll (one atomic Acquire load)
    // Wait()        — spin-wait with CPU_PAUSE; call only from non-worker
    //                 threads to avoid starving the pool when the queue empties
    //
    // -------------------------------------------------------------------------
    struct JobHandle {
        SharedPtr<JobCompletion> mCompletion;

        bool IsValid() const {
            return mCompletion != nullptr;
        }

        bool IsComplete() const {
            return mCompletion && mCompletion->mPendingCount.Load(EMemoryOrder::Acquire) == 0;
        }

        // Spin until all pending work is done.
        // Uses UMBRA_CPU_PAUSE (from Mutex.h) to reduce power and pipeline pressure.
        void Wait() const {
            if (!mCompletion) {
                return;
            }
            while (mCompletion->mPendingCount.Load(EMemoryOrder::Acquire) != 0) {
                UMBRA_CPU_PAUSE();
            }
        }

        static JobHandle Invalid() {
            return JobHandle{};
        }
    };

} // namespace Umbra
