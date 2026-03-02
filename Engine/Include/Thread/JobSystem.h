#pragma once
#include "EnginePCH.h"
#include "Thread/ConditionVariable.h"
#include "Thread/JobHandle.h"
#include "Thread/Mutex.h"
#include "Thread/Thread.h"

namespace Umbra {

    // =========================================================================
    // JobSystem — fixed-size thread pool with completion tracking,
    //             dependency ordering, and parallel-for.
    //
    // USAGE EXAMPLE:
    //   auto* js = ServiceLocator::GetJobSystem();
    //
    //   JobHandle h = js->Submit([](){ DoWork(); });
    //   h.Wait();
    //
    //   JobHandle a = js->Submit(TaskA);
    //   JobHandle b = js->SubmitAfter(a, TaskB);
    //   JobHandle c = js->SubmitAfter(b, TaskC);
    //   c.Wait();
    //
    //   Vector<int> data(1000);
    //   js->ParallelFor(1000, [&](uint32 i){ data[i] = i * i; }).Wait();
    //
    // =========================================================================
    class JobSystem {
    public:
        // _threadCount == 0 → hardware_concurrency - 1 (minimum 1)
        explicit JobSystem(uint32 _threadCount = 0);
        ~JobSystem();

        JobSystem(const JobSystem&)            = delete;
        JobSystem& operator=(const JobSystem&) = delete;

        //  Submit a task; returns a handle to track completion.
        JobHandle Submit(std::function<void()> _task);

        // Submit a task that runs only after _parent has completed.
        // If _parent is already complete the job is enqueued immediately.
        // If _parent is invalid, equivalent to Submit(_task).
        JobHandle SubmitAfter(JobHandle _parent, std::function<void()> _task);

        // Fork _count items across all workers; returns a handle that
        // completes when every item has been processed. Items may execute
        // in any order and on any worker thread.
        JobHandle ParallelFor(uint32 _count, std::function<void(uint32)> _itemTask);

        // Signal shutdown and block until all worker threads have exited.
        // Safe to call multiple times (guarded internally).
        void Shutdown();

        // ── Inspection ──────────────────────────────────────────────────────
        uint32 GetWorkerCount() const {
            return static_cast<uint32>(mWorkers.size());
        }
        uint32 GetQueueDepth() const;
        bool IsWorkerBusy(uint32 _index) const;
        uint64 GetTotalJobsCompleted() const {
            return mTotalJobsCompleted.Load(EMemoryOrder::Relaxed);
        }

    private:
        // main worker loop (run on each Thread)
        void WorkerLoop(uint32 _workerIndex);

        // Internal enqueue: wraps a pre-built callable (already handles
        // completion-counter decrement and dependency wakeup) into the queue.
        void EnqueueRaw(std::function<void()> _rawTask);

        // thread pool
        Vector<UniquePtr<Thread>> mWorkers;

        // Per-worker busy flag — main thread reads for ImGui display
        Vector<UniquePtr<AtomicBool>> mWorkerBusy;

        // job queue guarded by mQueueMutex + mWorkReady CV
        Queue<std::function<void()>> mJobQueue;
        mutable Mutex mQueueMutex;
        ConditionVariable mWorkReady;

        // Approximate queue depth for lock-free inspection from the main thread
        AtomicInt32 mApproxQueueDepth{0};

        //  shutdown flag; set before NotifyAll() to wake workers cleanly
        AtomicBool mShutdown{false};

        //  monotonic counter for throughput graphs
        AtomicUInt64 mTotalJobsCompleted{0};
    };

} // namespace Umbra
