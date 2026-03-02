#include "Thread/JobSystem.h"

#include <thread> // for std::thread::hardware_concurrency

namespace Umbra {

    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    JobSystem::JobSystem(uint32 _threadCount) {
        if (_threadCount == 0) {
            uint32 hw    = static_cast<uint32>(std::thread::hardware_concurrency());
            _threadCount = hw > 1 ? hw - 1 : 1;
        }

        mWorkers.reserve(_threadCount);
        mWorkerBusy.reserve(_threadCount);

        for (uint32 i = 0; i < _threadCount; ++i) {
            // Allocate busy-flag before starting the thread so WorkerLoop
            // can safely read mWorkerBusy[i] on first iteration.
            mWorkerBusy.push_back(std::make_unique<AtomicBool>(false));

            String name = String("JobWorker-") + std::to_string(i);
            auto thread = std::make_unique<Thread>(name);
            thread->Start(&JobSystem::WorkerLoop, this, i);
            mWorkers.push_back(std::move(thread));
        }
    }

    JobSystem::~JobSystem() {
        Shutdown();
    }

    // =========================================================================
    // Shutdown
    //
    // Uses Exchange(true) as a one-shot guard so calling Shutdown() from
    // the destructor is safe even if the user already called it manually.
    // =========================================================================

    void JobSystem::Shutdown() {
        // Exchange returns the old value; if it was already true we're done.
        if (mShutdown.Exchange(true)) {
            return;
        }

        // Wake all sleeping workers.  Those with pending jobs drain the queue;
        // those with an empty queue exit their WorkerLoop immediately.
        mWorkReady.NotifyAll();

        for (auto& worker : mWorkers) {
            if (worker && worker->IsJoinable()) {
                worker->Join();
            }
        }
        // mWorkers intentionally not cleared — GetWorkerCount() stays accurate
        // for post-shutdown inspection (e.g. ImGui debug panel).
    }

    // =========================================================================
    // Worker Loop
    // Each worker waits on mWorkReady until either:
    //   (a) a job is available in mJobQueue, or
    //   (b) mShutdown is set.
    // After (b) workers drain whatever remains in the queue before exiting,
    // because the predicate continues to return true while the queue is non-empty.
    // =========================================================================

    void JobSystem::WorkerLoop(uint32 _workerIndex) {
        AtomicBool* busyFlag = mWorkerBusy[_workerIndex].get();

        while (true) {
            std::function<void()> task;
            // locked dequeue of task
            {
                UniqueLock lock(mQueueMutex);

                // Sleep until there is work OR we should shut down.
                // Spurious wakeups are handled by the predicate loop inside Wait().
                mWorkReady.Wait(lock, [&] { return !mJobQueue.empty() || mShutdown.Load(EMemoryOrder::Acquire); });

                // If queue is empty at this point, shutdown is set — exit.
                if (mJobQueue.empty()) {
                    break;
                }

                task = std::move(mJobQueue.front());
                mJobQueue.pop();
                mApproxQueueDepth.FetchSub(1, EMemoryOrder::Relaxed);
            } // release queue lock before executing the task

            busyFlag->Store(true, EMemoryOrder::Release);
            task(); // execute — may fire dependency callbacks (see M3)
            busyFlag->Store(false, EMemoryOrder::Release);

            mTotalJobsCompleted.FetchAdd(1, EMemoryOrder::Relaxed);
        }
    }

    // =========================================================================
    // Internal helpers
    // =========================================================================

    void JobSystem::EnqueueRaw(std::function<void()> _rawTask) {
        {
            LockGuard<Mutex> lock(mQueueMutex);
            mJobQueue.push(std::move(_rawTask));
            mApproxQueueDepth.FetchAdd(1, EMemoryOrder::Relaxed);
        }
        mWorkReady.NotifyOne();
    }

    // =========================================================================
    //  Submit
    //
    // Wraps the caller's task in a lambda that also decrements the shared
    // completion counter when done.  The counter starts at 1 so the first
    // (and only) decrement hits 0 and fires any registered callbacks.
    // =========================================================================

    JobHandle JobSystem::Submit(std::function<void()> _task) {
        auto completion = std::make_shared<JobCompletion>();
        // mPendingCount defaults to 1 (see JobCompletion ctor)

        EnqueueRaw([task = std::move(_task), completion]() {
            task();
            completion->Decrement();
        });

        return JobHandle{completion};
    }

    // =========================================================================
    // SubmitAfter (dependency chain)
    //
    // The child job should only enter the queue once _parent completes.
    // We take the parent's callback lock to prevent a race between
    // Decrement() (which fires callbacks) and AddCallback():
    //
    //   Thread A (worker finishing parent):
    //     FetchSub → hits 0 → acquires lock → fires callbacks
    //
    //   Thread B (SubmitAfter caller):
    //     acquires lock → checks mPendingCount (still > 0) → appends callback
    //
    // Because both sides hold the same lock, one of two outcomes:
    //   (i)  B appends before A fires → callback runs when parent finishes ✓
    //   (ii) A fires (clears list) before B → B sees count == 0 (fast path) ✓
    // =========================================================================

    JobHandle JobSystem::SubmitAfter(JobHandle _parent, std::function<void()> _task) {
        auto completion = std::make_shared<JobCompletion>();
        JobHandle handle{completion};

        // No dependency — submit immediately, same as Submit().
        if (!_parent.IsValid()) {
            EnqueueRaw([task = std::move(_task), completion]() {
                task();
                completion->Decrement();
            });
            return handle;
        }

        bool enqueueNow = false;
        {
            LockGuard<Mutex> lock(_parent.mCompletion->mCallbackMutex);

            if (_parent.mCompletion->mPendingCount.Load(EMemoryOrder::Acquire) == 0) {
                // Parent already complete — we'll enqueue after releasing the lock.
                enqueueNow = true;
            } else {
                // Parent still running — register a callback that will enqueue us.
                _parent.mCompletion->mCallbacks.push_back([this, task = std::move(_task), completion]() mutable {
                    this->EnqueueRaw([task = std::move(task), completion]() mutable {
                        task();
                        completion->Decrement();
                    });
                });
            }
        }

        if (enqueueNow) {
            EnqueueRaw([task = std::move(_task), completion]() mutable {
                task();
                completion->Decrement();
            });
        }

        return handle;
    }

    // =========================================================================
    //  ParallelFor (fork-join)
    //
    // Creates ONE shared JobCompletion initialized to _count. Each of the
    // _count items gets its own queue entry; every entry calls Decrement()
    // when done.  The returned JobHandle is complete when all N decrements
    // have fired (mPendingCount == 0).
    //
    // All N items are pushed under a single lock acquisition then NotifyAll()
    // wakes every idle worker simultaneously — minimal lock contention.
    // =========================================================================

    JobHandle JobSystem::ParallelFor(uint32 _count, std::function<void(uint32)> _itemTask) {
        auto completion = std::make_shared<JobCompletion>();

        if (_count == 0) {
            completion->mPendingCount.Store(0);
            return JobHandle{completion};
        }

        // Override the default pending count (1) with the actual item count.
        completion->mPendingCount.Store(static_cast<int32>(_count));

        {
            LockGuard<Mutex> lock(mQueueMutex);
            for (uint32 i = 0; i < _count; ++i) {
                mJobQueue.push([_itemTask, i, completion]() {
                    _itemTask(i);
                    completion->Decrement();
                });
            }
            mApproxQueueDepth.FetchAdd(static_cast<int32>(_count), EMemoryOrder::Relaxed);
        }
        // Wake all workers at once — each will grab one item.
        mWorkReady.NotifyAll();

        return JobHandle{completion};
    }

    // =========================================================================
    // Inspection
    // =========================================================================

    uint32 JobSystem::GetQueueDepth() const {
        int32 depth = mApproxQueueDepth.Load(EMemoryOrder::Relaxed);
        return depth > 0 ? static_cast<uint32>(depth) : 0u;
    }

    bool JobSystem::IsWorkerBusy(uint32 _index) const {
        if (_index >= static_cast<uint32>(mWorkerBusy.size())) {
            return false;
        }
        return mWorkerBusy[_index]->Load(EMemoryOrder::Acquire);
    }

} // namespace Umbra
