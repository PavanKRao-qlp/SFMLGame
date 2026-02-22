#pragma once
#include "Game/Scene.h"
#include "Thread/ConditionVariable.h"
#include "Thread/JobHandle.h"
#include "Thread/Mutex.h"
#include "Thread/Thread.h"

class ThreadDemoScene : public Umbra::Scene {
public:
    ThreadDemoScene();
    virtual ~ThreadDemoScene() override;

    virtual void Initialize() override;
    virtual void OnBeginPlay() override;
    virtual void OnEndPlay() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    virtual void ShutDown() override;

private:
    // ---- Mutex / SpinLock demo ---------------------------------------------
    void StartThreads();
    void StopThreads();
    void WorkerFunction(int _threadIdx);
    void DrawMutexSection();

    static constexpr int THREAD_COUNT = 4;

    Umbra::Vector<Umbra::UniquePtr<Umbra::Thread>> mThreads;

    Umbra::AtomicUInt64 mIterationCounts[THREAD_COUNT];
    Umbra::AtomicBool   mWaitingForMutex[THREAD_COUNT];
    Umbra::AtomicBool   mWaitingForSpinLock[THREAD_COUNT];

    volatile int64_t mUnprotectedCounter{0};
    volatile int64_t mMutexCounter{0};
    volatile int64_t mSpinLockCounter{0};

    Umbra::Mutex     mSharedMutex;
    Umbra::SpinLock  mSharedSpinLock;

    Umbra::AtomicBool mShouldStop{false};
    bool              mThreadsRunning{false};

    // ---- Condition Variable demo -------------------------------------------
    void StartCVDemo();
    void StopCVDemo();
    void ProducerFunction();
    void ConsumerFunction();
    void DrawCVSection();

    Umbra::ConditionVariable        mCV;
    Umbra::Mutex                    mCVMutex;
    Umbra::Vector<int>              mCVQueue;       // guarded by mCVMutex
    Umbra::AtomicBool               mCVShouldStop{false};
    Umbra::AtomicBool               mProducerSleeping{false};
    Umbra::AtomicBool               mConsumerWaiting{false};
    Umbra::AtomicUInt64             mProducedCount{0};
    Umbra::AtomicUInt64             mConsumedCount{0};
    Umbra::UniquePtr<Umbra::Thread> mProducerThread;
    Umbra::UniquePtr<Umbra::Thread> mConsumerThread;
    bool                            mCVRunning{false};

    // ---- Job System demo ---------------------------------------------------
    void DrawJobSystemSection();
    void DrawJsPoolPanel();
    void DrawJsParallelForPanel();
    void DrawJsDependencyChainPanel();
    void DrawJsThroughputPanel();
    void UpdateJsThroughput(double _now);
    void LaunchJsBenchmark(Umbra::uint32 _count);
    void LaunchJsChain();

    // Throughput graph — circular buffer sampled at ~10 Hz
    static constexpr int JS_GRAPH_SAMPLES = 80;
    float                mJsThroughputHistory[JS_GRAPH_SAMPLES]{};
    int                  mJsGraphOffset{0};
    Umbra::uint64        mJsLastJobCount{0};
    double               mJsLastSampleTime{-1.0};
    float                mJsPeakThroughput{1.0f};

    // ParallelFor benchmark
    Umbra::AtomicBool            mJsBenchmarkRunning{false};
    Umbra::Vector<Umbra::uint32> mJsBenchmarkResults;
    float                        mJsBenchmarkMs{-1.0f};
    Umbra::JobHandle             mJsBenchmarkHandle;

    // Dependency chain A -> B -> C
    Umbra::AtomicBool    mJsChainRunning{false};
    Umbra::Atomic<float> mJsTsA{-1.0f};   // ms since chain launch (-1 = pending)
    Umbra::Atomic<float> mJsTsB{-1.0f};
    Umbra::Atomic<float> mJsTsC{-1.0f};
    double               mJsChainLaunchTime{0.0};

    // ---- Shared UI ---------------------------------------------------------
    void DrawUI();
};
