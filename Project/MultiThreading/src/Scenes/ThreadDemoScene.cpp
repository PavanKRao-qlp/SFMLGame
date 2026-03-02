#include "ThreadDemoScene.h"
#include "Service/ServiceLocator.h"

#include "imgui.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>

static constexpr float THREAD_HUES[4]        = {0.00f, 0.33f, 0.55f, 0.72f};
static constexpr const char* THREAD_NAMES[4] = {"Alpha", "Beta", "Gamma", "Delta"};

// ---------------------------------------------------------------------------

ThreadDemoScene::ThreadDemoScene() {
    for (int i = 0; i < THREAD_COUNT; i++) {
        mIterationCounts[i].Store(0);
        mWaitingForMutex[i].Store(false);
        mWaitingForSpinLock[i].Store(false);
    }
}

ThreadDemoScene::~ThreadDemoScene() {
    StopThreads();
    StopCVDemo();
}

void ThreadDemoScene::Initialize() {
    CreateDefaultCamera(100.0f);
}

void ThreadDemoScene::OnBeginPlay() {
    StartThreads();
    // Reset JS throughput baseline so the graph starts from 0
    mJsLastSampleTime = -1.0;
    if (Umbra::ServiceLocator::GetJobSystem()) {
        mJsLastJobCount = Umbra::ServiceLocator::GetJobSystem()->GetTotalJobsCompleted();
    }
}

void ThreadDemoScene::OnEndPlay() {
    StopThreads();
    StopCVDemo();
}

void ThreadDemoScene::OnFixedUpdate() {}

void ThreadDemoScene::OnUpdate() {
    UpdateJsThroughput(ImGui::GetTime());
    DrawUI();
}

void ThreadDemoScene::ShutDown() {
    StopThreads();
    StopCVDemo();
}

Umbra::SharedPtr<Umbra::Scene> ThreadDemoScene::InstantiateCopy() {
    return std::make_shared<ThreadDemoScene>();
}

// ---------------------------------------------------------------------------
// Mutex / SpinLock demo — thread management
// ---------------------------------------------------------------------------

void ThreadDemoScene::StartThreads() {
    if (mThreadsRunning) {
        return;
    }

    mShouldStop.Store(false);
    mUnprotectedCounter = 0;
    mMutexCounter       = 0;
    mSpinLockCounter    = 0;

    for (int i = 0; i < THREAD_COUNT; i++) {
        mIterationCounts[i].Store(0);
        mWaitingForMutex[i].Store(false);
        mWaitingForSpinLock[i].Store(false);
    }

    mThreads.clear();
    for (int i = 0; i < THREAD_COUNT; i++) {
        Umbra::String name = Umbra::String("Worker-") + THREAD_NAMES[i];
        auto thread        = std::make_unique<Umbra::Thread>(name);
        thread->Start(&ThreadDemoScene::WorkerFunction, this, i);
        mThreads.push_back(std::move(thread));
    }

    mThreadsRunning = true;
}

void ThreadDemoScene::StopThreads() {
    if (!mThreadsRunning) {
        return;
    }

    mShouldStop.Store(true);
    for (auto& thread : mThreads) {
        if (thread && thread->IsJoinable()) {
            thread->Join();
        }
    }
    mThreads.clear();
    mThreadsRunning = false;
}

// ---------------------------------------------------------------------------
// Mutex / SpinLock demo — worker
// ---------------------------------------------------------------------------

void ThreadDemoScene::WorkerFunction(int _threadIdx) {
    while (!mShouldStop.Load(Umbra::EMemoryOrder::Acquire)) {
        mIterationCounts[_threadIdx].FetchAdd(1, Umbra::EMemoryOrder::Relaxed);

        // ---- 1. UNPROTECTED (data race — intentional) ----------------------
        {
            int64_t val = mUnprotectedCounter;
            std::this_thread::yield();
            mUnprotectedCounter = val + 1;
        }

        // ---- 2. MUTEX — OS-level blocking lock -----------------------------
        {
            mWaitingForMutex[_threadIdx].Store(true, Umbra::EMemoryOrder::Release);
            Umbra::LockGuard<Umbra::Mutex> lock(mSharedMutex);
            mWaitingForMutex[_threadIdx].Store(false, Umbra::EMemoryOrder::Release);
            mMutexCounter++;
        }

        // ---- 3. SPINLOCK — user-space busy-wait lock -----------------------
        {
            mWaitingForSpinLock[_threadIdx].Store(true, Umbra::EMemoryOrder::Release);
            Umbra::LockGuard<Umbra::SpinLock> lock(mSharedSpinLock);
            mWaitingForSpinLock[_threadIdx].Store(false, Umbra::EMemoryOrder::Release);
            mSpinLockCounter++;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// ---------------------------------------------------------------------------
// Condition Variable demo — thread management
// ---------------------------------------------------------------------------

void ThreadDemoScene::StartCVDemo() {
    if (mCVRunning) {
        return;
    }

    mCVShouldStop.Store(false);
    mProducedCount.Store(0);
    mConsumedCount.Store(0);
    mProducerSleeping.Store(false);
    mConsumerWaiting.Store(false);
    mCVQueue.clear();

    mProducerThread = std::make_unique<Umbra::Thread>("CV-Producer");
    mProducerThread->Start(&ThreadDemoScene::ProducerFunction, this);

    mConsumerThread = std::make_unique<Umbra::Thread>("CV-Consumer");
    mConsumerThread->Start(&ThreadDemoScene::ConsumerFunction, this);

    mCVRunning = true;
}

void ThreadDemoScene::StopCVDemo() {
    if (!mCVRunning) {
        return;
    }

    mCVShouldStop.Store(true);
    mCV.NotifyAll(); // wake the consumer if it is sleeping on the CV

    if (mProducerThread && mProducerThread->IsJoinable()) {
        mProducerThread->Join();
    }
    if (mConsumerThread && mConsumerThread->IsJoinable()) {
        mConsumerThread->Join();
    }

    mProducerThread.reset();
    mConsumerThread.reset();
    mCVRunning = false;
}

// ---------------------------------------------------------------------------
// Condition Variable demo — producer
//
// Sleeps 2s then pushes BURST_SIZE items all at once, then notifies.
// This creates a visible window where the consumer waits (queue empty)
// followed by a visible window where the queue drains item by item.
// ---------------------------------------------------------------------------

static constexpr int CV_BURST_SIZE = 5;

void ThreadDemoScene::ProducerFunction() {
    int nextValue = 1;
    while (!mCVShouldStop.Load()) {
        // Sleep 2000ms in 50ms chunks — allows clean, fast shutdown
        for (int chunk = 0; chunk < 40 && !mCVShouldStop.Load(); ++chunk) {
            mProducerSleeping.Store(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        mProducerSleeping.Store(false);

        if (mCVShouldStop.Load()) {
            break;
        }

        // Push a burst of items under one lock acquisition
        {
            Umbra::LockGuard<Umbra::Mutex> lock(mCVMutex);
            for (int i = 0; i < CV_BURST_SIZE; ++i) {
                mCVQueue.push_back(nextValue++);
            }
        }

        mProducedCount.FetchAdd(CV_BURST_SIZE);
        mCV.NotifyOne(); // wake the consumer — it will drain one-by-one
    }
}

// ---------------------------------------------------------------------------
// Condition Variable demo — consumer
//
// Blocks on the CV until the producer pushes an item. When woken, pops the
// item, releases the lock, then simulates 300ms of processing work.
// ---------------------------------------------------------------------------

void ThreadDemoScene::ConsumerFunction() {
    while (!mCVShouldStop.Load()) {
        int item = -1;

        {
            Umbra::UniqueLock lock(mCVMutex);

            // Atomically: release the mutex, sleep, reacquire on wakeup.
            // The predicate guards against spurious wakeups.
            mConsumerWaiting.Store(true);
            mCV.Wait(lock, [&] { return !mCVQueue.empty() || mCVShouldStop.Load(); });
            mConsumerWaiting.Store(false);

            if (mCVShouldStop.Load()) {
                break;
            }

            item = mCVQueue.front();
            mCVQueue.erase(mCVQueue.begin());
        } // lock released — consumer processes outside the critical section

        // Simulate work — no lock held during this
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        mConsumedCount.FetchAdd(1);
    }
}

// ---------------------------------------------------------------------------
// UI
// ---------------------------------------------------------------------------

void ThreadDemoScene::DrawUI() {
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos({0.0f, 0.0f}, ImGuiCond_Always);
    ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
    ImGui::Begin("##ThreadDemo", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::TextColored({1.0f, 0.85f, 0.2f, 1.0f}, "UMBRA Engine  :  Thread & Synchronization Demonstration");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("##sections")) {
        if (ImGui::BeginTabItem("Mutex & SpinLock")) {
            DrawMutexSection();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Condition Variable")) {
            DrawCVSection();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Job System")) {
            DrawJobSystemSection();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

// ---------------------------------------------------------------------------
// Mutex / SpinLock section
// ---------------------------------------------------------------------------

void ThreadDemoScene::DrawMutexSection() {
    ImGui::TextColored({1.0f, 0.85f, 0.2f, 1.0f}, "Mutex & SpinLock");
    ImGui::Spacing();

    // Controls
    if (!mThreadsRunning) {
        if (ImGui::Button("Start Threads", {130.0f, 28.0f})) {
            StartThreads();
        }
    } else {
        if (ImGui::Button("Stop Threads", {130.0f, 28.0f})) {
            StopThreads();
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart", {80.0f, 28.0f})) {
            StopThreads();
            StartThreads();
        }
    }

    ImGui::Spacing();

    // ---- Thread status bars ------------------------------------------------
    ImGui::TextColored({0.7f, 0.9f, 1.0f, 1.0f}, "Thread Status");
    ImGui::Spacing();

    ImDrawList* dl    = ImGui::GetWindowDrawList();
    ImVec2 cursor     = ImGui::GetCursorScreenPos();
    float avail       = ImGui::GetContentRegionAvail().x;
    const float kBarH = 52.0f;
    const float kGap  = 8.0f;
    float barW        = (avail - kGap * (THREAD_COUNT - 1)) / (float) THREAD_COUNT;
    double t          = ImGui::GetTime();

    for (int i = 0; i < THREAD_COUNT; i++) {
        float x = cursor.x + i * (barW + kGap);
        float y = cursor.y;

        float r, g, b;
        ImVec4 col;
        if (!mThreadsRunning) {
            col = {0.22f, 0.22f, 0.22f, 1.0f};
        } else if (mWaitingForMutex[i].Load() || mWaitingForSpinLock[i].Load()) {
            col = {1.0f, 0.60f, 0.0f, 1.0f};
        } else {
            ImGui::ColorConvertHSVtoRGB(THREAD_HUES[i], 0.70f, 0.85f, r, g, b);
            col = {r, g, b, 1.0f};
        }
        uint32_t colU32 = ImGui::ColorConvertFloat4ToU32(col);
        dl->AddRectFilled({x, y}, {x + barW, y + kBarH}, colU32, 6.0f);

        if (mThreadsRunning && !mWaitingForMutex[i].Load() && !mWaitingForSpinLock[i].Load()) {
            float pulse = (float) (0.5 + 0.5 * sin(t * 3.5 + i * 1.6));
            ImVec4 glow = col;
            glow.w      = 0.4f * pulse;
            dl->AddRect({x - 3.0f, y - 3.0f}, {x + barW + 3.0f, y + kBarH + 3.0f}, ImGui::ColorConvertFloat4ToU32(glow),
                6.0f, 0, 3.5f);
        }

        ImVec2 ts = ImGui::CalcTextSize(THREAD_NAMES[i]);
        float tx  = x + (barW - ts.x) * 0.5f;
        float ty  = y + (kBarH - ts.y) * 0.5f - 7.0f;
        dl->AddText({tx, ty}, IM_COL32(255, 255, 255, 255), THREAD_NAMES[i]);

        const char* stateStr;
        if (!mThreadsRunning) {
            stateStr = "Idle";
        } else if (mWaitingForMutex[i].Load()) {
            stateStr = "Waiting (Mutex)";
        } else if (mWaitingForSpinLock[i].Load()) {
            stateStr = "Waiting (SpinLock)";
        } else {
            stateStr = "Running";
        }

        ImVec2 ss = ImGui::CalcTextSize(stateStr);
        dl->AddText({x + (barW - ss.x) * 0.5f, ty + ts.y + 3.0f}, IM_COL32(220, 220, 220, 210), stateStr);
    }
    ImGui::Dummy({avail, kBarH + 6.0f});

    ImGui::Text("Legend:");
    ImGui::SameLine();
    ImGui::TextColored({0.3f, 0.85f, 0.3f, 1.0f}, "Running");
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::TextColored({1.0f, 0.60f, 0.0f, 1.0f}, "Waiting for Lock");
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::TextColored({0.45f, 0.45f, 0.45f, 1.0f}, "Idle");

    ImGui::Spacing();

    // ---- Per-thread statistics table ---------------------------------------
    ImGui::TextColored({0.7f, 0.9f, 1.0f, 1.0f}, "Thread Statistics");
    ImGui::Spacing();

    if (ImGui::BeginTable(
            "##stats", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableSetupColumn("Thread");
        ImGui::TableSetupColumn("Iterations");
        ImGui::TableSetupColumn("Status");
        ImGui::TableHeadersRow();

        for (int i = 0; i < THREAD_COUNT; i++) {
            ImGui::TableNextRow();
            float r2, g2, b2;
            ImGui::ColorConvertHSVtoRGB(THREAD_HUES[i], 0.70f, 0.85f, r2, g2, b2);

            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored({r2, g2, b2, 1.0f}, "%s", THREAD_NAMES[i]);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%lld", (long long) mIterationCounts[i].Load());

            ImGui::TableSetColumnIndex(2);
            if (!mThreadsRunning) {
                ImGui::TextColored({0.5f, 0.5f, 0.5f, 1.0f}, "Idle");
            } else if (mWaitingForMutex[i].Load()) {
                ImGui::TextColored({1.0f, 0.60f, 0.0f, 1.0f}, "Waiting (Mutex)");
            } else if (mWaitingForSpinLock[i].Load()) {
                ImGui::TextColored({1.0f, 0.60f, 0.0f, 1.0f}, "Waiting (SpinLock)");
            } else {
                ImGui::TextColored({0.3f, 0.85f, 0.3f, 1.0f}, "Running");
            }
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();

    // ---- Counter comparison ------------------------------------------------
    ImGui::TextColored({0.7f, 0.9f, 1.0f, 1.0f}, "Shared Counter Comparison");
    ImGui::Spacing();

    int64_t expected = 0;
    for (int i = 0; i < THREAD_COUNT; i++) {
        expected += mIterationCounts[i].Load(Umbra::EMemoryOrder::Relaxed);
    }

    int64_t unprotectedVal = mUnprotectedCounter;
    int64_t mutexVal       = mMutexCounter;
    int64_t spinVal        = mSpinLockCounter;
    int64_t lostUpdates    = expected - unprotectedVal;

    // Column 1 — Unprotected
    ImGui::BeginGroup();
    ImGui::TextColored({1.0f, 0.35f, 0.35f, 1.0f}, "Unprotected");
    ImGui::TextColored({1.0f, 0.35f, 0.35f, 1.0f}, "(Race Condition)");
    ImGui::Text("Value:    %lld", (long long) unprotectedVal);
    ImGui::Text("Expected: %lld", (long long) expected);
    if (lostUpdates > 0) {
        ImGui::TextColored({1.0f, 0.2f, 0.2f, 1.0f}, "! Lost %lld updates", (long long) lostUpdates);
    } else {
        ImGui::TextColored({1.0f, 0.75f, 0.0f, 1.0f}, "~ No race yet");
    }
    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 16.0f);

    // Column 2 — Mutex
    ImGui::BeginGroup();
    ImGui::TextColored({0.35f, 0.85f, 0.35f, 1.0f}, "Mutex");
    ImGui::TextColored({0.35f, 0.85f, 0.35f, 1.0f}, "(OS Blocking Lock)");
    ImGui::Text("Value:    %lld", (long long) mutexVal);
    ImGui::Text("Expected: %lld", (long long) expected);
    if (mutexVal >= expected && expected > 0) {
        ImGui::TextColored({0.3f, 1.0f, 0.3f, 1.0f}, "OK Correct!");
    } else {
        ImGui::TextColored({0.7f, 0.7f, 0.7f, 1.0f}, "Syncing...");
    }
    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 16.0f);

    // Column 3 — SpinLock
    ImGui::BeginGroup();
    ImGui::TextColored({0.35f, 0.75f, 1.0f, 1.0f}, "SpinLock");
    ImGui::TextColored({0.35f, 0.75f, 1.0f, 1.0f}, "(Busy-Wait Lock)");
    ImGui::Text("Value:    %lld", (long long) spinVal);
    ImGui::Text("Expected: %lld", (long long) expected);
    if (spinVal >= expected && expected > 0) {
        ImGui::TextColored({0.35f, 0.75f, 1.0f, 1.0f}, "OK Correct!");
    } else {
        ImGui::TextColored({0.7f, 0.7f, 0.7f, 1.0f}, "Syncing...");
    }
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::TextColored({0.8f, 0.8f, 0.8f, 1.0f}, "How It Works:");
    ImGui::Spacing();
    ImGui::TextWrapped("Unprotected: Each thread reads the counter value, yields the CPU (so another "
                       "thread can slip in and read the same value), then writes counter+1. Both threads "
                       "end up writing the same value, discarding one of the increments. This is a classic "
                       "read-modify-write data race.");
    ImGui::Spacing();
    ImGui::TextWrapped("Mutex (OS Blocking): Before touching the counter a thread acquires an OS-level "
                       "lock. If the lock is taken, the OS suspends the thread and wakes it later. Only "
                       "one thread is inside the critical section at a time. Overhead: ~100-1000 ns per "
                       "acquisition due to the kernel/user-space transition.");
    ImGui::Spacing();
    ImGui::TextWrapped("SpinLock (Busy-Wait): Uses a single atomic_flag. A waiting thread loops calling "
                       "test_and_set() until it wins the lock — no OS involvement, no sleep. Lower latency "
                       "for very short critical sections, but burns CPU cycles while spinning. Best on "
                       "multi-core machines with rare contention.");
}

// ---------------------------------------------------------------------------
// Condition Variable section
// ---------------------------------------------------------------------------

void ThreadDemoScene::DrawCVSection() {
    ImGui::TextColored({1.0f, 0.85f, 0.2f, 1.0f}, "Condition Variable — Producer / Consumer");
    ImGui::Spacing();

    // Controls
    if (!mCVRunning) {
        if (ImGui::Button("Start CV Demo", {140.0f, 28.0f})) {
            StartCVDemo();
        }
    } else {
        if (ImGui::Button("Stop CV Demo", {140.0f, 28.0f})) {
            StopCVDemo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart##cv", {80.0f, 28.0f})) {
            StopCVDemo();
            StartCVDemo();
        }
    }

    ImGui::Spacing();

    // ---- Thread state bars (Producer + Consumer) ---------------------------
    ImGui::TextColored({0.7f, 0.9f, 1.0f, 1.0f}, "Thread States");
    ImGui::Spacing();

    ImDrawList* dl    = ImGui::GetWindowDrawList();
    ImVec2 cursor     = ImGui::GetCursorScreenPos();
    float avail       = ImGui::GetContentRegionAvail().x;
    const float kBarH = 52.0f;
    float barW        = (avail - 8.0f) * 0.5f;
    double t          = ImGui::GetTime();

    // --- Producer bar ---
    {
        bool producerSleeping = mProducerSleeping.Load();

        ImVec4 col;
        if (!mCVRunning) {
            col = {0.22f, 0.22f, 0.22f, 1.0f};
        } else if (producerSleeping) {
            col = {0.25f, 0.45f, 0.85f, 1.0f}; // blue — sleeping
        } else {
            col = {0.25f, 0.85f, 0.45f, 1.0f}; // green — producing
        }

        uint32_t colU32 = ImGui::ColorConvertFloat4ToU32(col);
        dl->AddRectFilled(cursor, {cursor.x + barW, cursor.y + kBarH}, colU32, 6.0f);

        // Pulse glow when producing
        if (mCVRunning && !producerSleeping) {
            float pulse = (float) (0.5 + 0.5 * sin(t * 6.0));
            ImVec4 glow = col;
            glow.w      = 0.5f * pulse;
            dl->AddRect({cursor.x - 3.0f, cursor.y - 3.0f}, {cursor.x + barW + 3.0f, cursor.y + kBarH + 3.0f},
                ImGui::ColorConvertFloat4ToU32(glow), 6.0f, 0, 3.5f);
        }

        const char* label = "Producer";
        const char* stateStr =
            !mCVRunning ? "Idle" : (producerSleeping ? "Sleeping (waiting to produce)" : "Producing");
        ImVec2 ts = ImGui::CalcTextSize(label);
        ImVec2 ss = ImGui::CalcTextSize(stateStr);
        float ty  = cursor.y + (kBarH - ts.y) * 0.5f - 7.0f;
        dl->AddText({cursor.x + (barW - ts.x) * 0.5f, ty}, IM_COL32(255, 255, 255, 255), label);
        dl->AddText({cursor.x + (barW - ss.x) * 0.5f, ty + ts.y + 3.0f}, IM_COL32(220, 220, 220, 210), stateStr);
    }

    // --- Consumer bar ---
    {
        float cx             = cursor.x + barW + 8.0f;
        bool consumerWaiting = mConsumerWaiting.Load();

        ImVec4 col;
        if (!mCVRunning) {
            col = {0.22f, 0.22f, 0.22f, 1.0f};
        } else if (consumerWaiting) {
            col = {1.0f, 0.60f, 0.0f, 1.0f}; // amber — sleeping on CV
        } else {
            col = {0.25f, 0.85f, 0.45f, 1.0f}; // green — processing
        }

        uint32_t colU32 = ImGui::ColorConvertFloat4ToU32(col);
        dl->AddRectFilled({cx, cursor.y}, {cx + barW, cursor.y + kBarH}, colU32, 6.0f);

        // Pulse glow when processing
        if (mCVRunning && !consumerWaiting) {
            float pulse = (float) (0.5 + 0.5 * sin(t * 3.5));
            ImVec4 glow = col;
            glow.w      = 0.4f * pulse;
            dl->AddRect({cx - 3.0f, cursor.y - 3.0f}, {cx + barW + 3.0f, cursor.y + kBarH + 3.0f},
                ImGui::ColorConvertFloat4ToU32(glow), 6.0f, 0, 3.5f);
        }

        const char* label    = "Consumer";
        const char* stateStr = !mCVRunning ? "Idle" : (consumerWaiting ? "Waiting on CV (asleep)" : "Processing item");
        ImVec2 ts            = ImGui::CalcTextSize(label);
        ImVec2 ss            = ImGui::CalcTextSize(stateStr);
        float ty             = cursor.y + (kBarH - ts.y) * 0.5f - 7.0f;
        dl->AddText({cx + (barW - ts.x) * 0.5f, ty}, IM_COL32(255, 255, 255, 255), label);
        dl->AddText({cx + (barW - ss.x) * 0.5f, ty + ts.y + 3.0f}, IM_COL32(220, 220, 220, 210), stateStr);
    }

    ImGui::Dummy({avail, kBarH + 6.0f});
    ImGui::Spacing();

    // ---- Queue visualization -----------------------------------------------
    ImGui::TextColored({0.7f, 0.9f, 1.0f, 1.0f}, "Shared Queue");
    ImGui::Spacing();

    // Snapshot the queue under lock so we don't race on display
    Umbra::Vector<int> snapshot;
    {
        Umbra::LockGuard<Umbra::Mutex> lock(mCVMutex);
        snapshot = mCVQueue;
    }

    ImGui::Text("Depth: %d", (int) snapshot.size());
    ImGui::SameLine(0.0f, 16.0f);
    ImGui::TextColored(
        {0.6f, 0.6f, 0.6f, 1.0f}, "(burst of %d items every ~2s  |  consumer: ~600ms each)", CV_BURST_SIZE);
    ImGui::Spacing();

    const int kMaxSlots   = 8;
    const float kSlotSize = 40.0f;
    const float kSlotGap  = 6.0f;
    ImVec2 qCursor        = ImGui::GetCursorScreenPos();

    for (int i = 0; i < kMaxSlots; ++i) {
        float sx = qCursor.x + i * (kSlotSize + kSlotGap);
        float sy = qCursor.y;

        bool filled    = i < (int) snapshot.size();
        ImVec4 slotCol = filled ? ImVec4{0.35f, 0.75f, 0.35f, 1.0f} : ImVec4{0.18f, 0.18f, 0.18f, 1.0f};

        dl->AddRectFilled({sx, sy}, {sx + kSlotSize, sy + kSlotSize}, ImGui::ColorConvertFloat4ToU32(slotCol), 4.0f);
        dl->AddRect({sx, sy}, {sx + kSlotSize, sy + kSlotSize}, IM_COL32(80, 80, 80, 200), 4.0f);

        if (filled) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%d", snapshot[i]);
            ImVec2 ts = ImGui::CalcTextSize(buf);
            dl->AddText(
                {sx + (kSlotSize - ts.x) * 0.5f, sy + (kSlotSize - ts.y) * 0.5f}, IM_COL32(255, 255, 255, 255), buf);
        }
    }

    if (snapshot.size() > kMaxSlots) {
        float ox = qCursor.x + kMaxSlots * (kSlotSize + kSlotGap);
        char more[16];
        snprintf(more, sizeof(more), "+%d", (int) snapshot.size() - kMaxSlots);
        dl->AddText(
            {ox, qCursor.y + (kSlotSize - ImGui::GetTextLineHeight()) * 0.5f}, IM_COL32(200, 200, 200, 200), more);
    }

    ImGui::Dummy({avail, kSlotSize + 6.0f});
    ImGui::Spacing();

    // ---- Stats -------------------------------------------------------------
    uint64_t produced = mProducedCount.Load();
    uint64_t consumed = mConsumedCount.Load();

    ImGui::BeginGroup();
    ImGui::TextColored({0.25f, 0.85f, 0.45f, 1.0f}, "Produced");
    ImGui::Text("%llu", (unsigned long long) produced);
    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 32.0f);

    ImGui::BeginGroup();
    ImGui::TextColored({0.35f, 0.75f, 0.35f, 1.0f}, "In Queue");
    ImGui::Text("%llu", (unsigned long long) snapshot.size());
    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 32.0f);

    ImGui::BeginGroup();
    ImGui::TextColored({0.35f, 0.75f, 1.0f, 1.0f}, "Consumed");
    ImGui::Text("%llu", (unsigned long long) consumed);
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ---- Explanation -------------------------------------------------------
    ImGui::TextColored({0.8f, 0.8f, 0.8f, 1.0f}, "How It Works:");
    ImGui::Spacing();
    ImGui::TextWrapped("Producer: Sleeps ~2s (queue empty, consumer blocks on CV — shown amber). Then "
                       "pushes a burst of 5 items under one lock and calls NotifyOne().");
    ImGui::Spacing();
    ImGui::TextWrapped("Consumer: Calls cv.Wait(lock, predicate) — atomically releases the mutex and "
                       "sleeps until notified. On wakeup it pops ONE item, releases the lock, then "
                       "processes for ~600ms. It loops back: if the queue still has items the predicate "
                       "is true and it skips the sleep, otherwise it blocks again. This drains the burst "
                       "one item at a time — the queue depth counts down visibly from 5 to 0.");
    ImGui::Spacing();
    ImGui::TextWrapped("Key insight: the consumer burns zero CPU while waiting. Unlike a SpinLock it does "
                       "not loop — it is truly sleeping until the OS wakes it via the notification.");
}

// ===========================================================================
// Job System section
// ===========================================================================

void ThreadDemoScene::DrawJobSystemSection() {
    Umbra::JobSystem* js = Umbra::ServiceLocator::GetJobSystem();
    if (!js) {
        ImGui::TextColored({1.0f, 0.3f, 0.3f, 1.0f}, "JobSystem not available from ServiceLocator.");
        return;
    }

    float halfW = (ImGui::GetContentRegionAvail().x - 12.0f) * 0.5f;

    // Left column — Pool Status + ParallelFor
    ImGui::BeginGroup();
    ImGui::PushItemWidth(halfW);
    DrawJsPoolPanel();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    DrawJsParallelForPanel();
    ImGui::PopItemWidth();
    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 12.0f);

    // Right column — Dependency Chain + Throughput
    ImGui::BeginGroup();
    ImGui::PushItemWidth(halfW);
    DrawJsDependencyChainPanel();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    DrawJsThroughputPanel();
    ImGui::PopItemWidth();
    ImGui::EndGroup();
}

// ---------------------------------------------------------------------------
// Job System — Thread Pool Status panel
// ---------------------------------------------------------------------------

void ThreadDemoScene::DrawJsPoolPanel() {
    ImGui::TextColored({1.0f, 0.85f, 0.2f, 1.0f}, "Thread Pool Status  (M1)");
    ImGui::Spacing();

    Umbra::JobSystem* js = Umbra::ServiceLocator::GetJobSystem();
    Umbra::uint32 workerCount = js->GetWorkerCount();
    Umbra::uint32 queueDepth  = js->GetQueueDepth();

    ImGui::Text("Workers: %u   |   Queue Depth: %u", workerCount, queueDepth);
    ImGui::Spacing();

    // Queue depth gauge — green → yellow → red
    float gaugeMax  = workerCount > 0 ? static_cast<float>(workerCount * 4) : 4.0f;
    float gaugeFrac = std::min(1.0f, static_cast<float>(queueDepth) / gaugeMax);
    ImVec4 gaugeCol;
    if (gaugeFrac < 0.5f) {
        gaugeCol = {gaugeFrac * 2.0f, 1.0f, 0.0f, 1.0f};
    } else {
        gaugeCol = {1.0f, 1.0f - (gaugeFrac - 0.5f) * 2.0f, 0.0f, 1.0f};
    }
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, gaugeCol);
    ImGui::ProgressBar(gaugeFrac, {-1.0f, 12.0f}, "");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextDisabled("Queue");
    ImGui::Spacing();

    // Per-worker bars
    ImGui::TextColored({0.7f, 0.9f, 1.0f, 1.0f}, "Worker Threads");
    ImGui::Spacing();

    ImDrawList* dl    = ImGui::GetWindowDrawList();
    ImVec2 cursor     = ImGui::GetCursorScreenPos();
    float avail       = ImGui::GetContentRegionAvail().x;
    const float kBarH = 44.0f;
    const float kGap  = 6.0f;
    float barW        = workerCount > 0
                            ? (avail - kGap * (workerCount - 1)) / static_cast<float>(workerCount)
                            : avail;
    double t          = ImGui::GetTime();

    for (Umbra::uint32 i = 0; i < workerCount; ++i) {
        bool  busy = js->IsWorkerBusy(i);
        float x    = cursor.x + i * (barW + kGap);

        ImVec4 col = busy
            ? ImVec4{0.25f, 0.85f, 0.45f, 1.0f}
            : ImVec4{0.22f, 0.22f, 0.22f, 1.0f};

        dl->AddRectFilled({x, cursor.y}, {x + barW, cursor.y + kBarH},
            ImGui::ColorConvertFloat4ToU32(col), 5.0f);

        if (busy) {
            float pulse = static_cast<float>(0.5 + 0.5 * sin(t * 4.0 + i * 1.3));
            ImVec4 glow = col;
            glow.w = 0.45f * pulse;
            dl->AddRect({x - 3.0f, cursor.y - 3.0f},
                        {x + barW + 3.0f, cursor.y + kBarH + 3.0f},
                        ImGui::ColorConvertFloat4ToU32(glow), 5.0f, 0, 3.0f);
        }

        char label[16];
        snprintf(label, sizeof(label), "W%u", i);
        ImVec2 ts = ImGui::CalcTextSize(label);
        dl->AddText({x + (barW - ts.x) * 0.5f, cursor.y + (kBarH - ts.y) * 0.5f - 6.0f},
                    IM_COL32(255, 255, 255, 255), label);

        const char* stateStr = busy ? "Working" : "Idle";
        ImVec2 ss = ImGui::CalcTextSize(stateStr);
        dl->AddText({x + (barW - ss.x) * 0.5f, cursor.y + (kBarH - ss.y) * 0.5f + 6.0f},
                    IM_COL32(210, 210, 210, 210), stateStr);
    }
    ImGui::Dummy({avail, kBarH + 6.0f});

    ImGui::Text("Legend:");
    ImGui::SameLine();
    ImGui::TextColored({0.25f, 0.85f, 0.45f, 1.0f}, "Working");
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::TextColored({0.45f, 0.45f, 0.45f, 1.0f}, "Idle");

    ImGui::Spacing();
    ImGui::TextColored({0.8f, 0.8f, 0.8f, 1.0f}, "How It Works:");
    ImGui::TextWrapped(
        "N worker threads block on a ConditionVariable. A producer calls "
        "Submit() or ParallelFor() — this locks the queue, pushes the job(s), "
        "then calls NotifyOne/NotifyAll. The woken worker pops one job, executes "
        "it, sets its busy flag back to false, then waits again. Workers burn "
        "zero CPU while idle.");
}

// ---------------------------------------------------------------------------
// Job System — ParallelFor benchmark panel
// ---------------------------------------------------------------------------

void ThreadDemoScene::LaunchJsBenchmark(Umbra::uint32 _count) {
    if (mJsBenchmarkRunning.Exchange(true)) return;

    mJsBenchmarkMs = -1.0f;
    mJsBenchmarkResults.assign(_count, 0u);

    auto startTime = std::chrono::high_resolution_clock::now();

    mJsBenchmarkHandle = Umbra::ServiceLocator::GetJobSystem()->ParallelFor(
        _count,
        [this](Umbra::uint32 _i) {
            // Deliberate 5 ms sleep so workers stay visibly green long enough
            // to see the pool panel update.  With N workers and 1000 items this
            // keeps all workers busy for roughly 5000/N ms (~700 ms at 7 workers).
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            mJsBenchmarkResults[_i] = _i * _i;
        });

    mJsBenchmarkHandle.mCompletion->AddCallback(
        [this, startTime]() {
            float ms = std::chrono::duration<float, std::milli>(
                           std::chrono::high_resolution_clock::now() - startTime).count();
            mJsBenchmarkMs = ms;
            mJsBenchmarkRunning.Store(false, Umbra::EMemoryOrder::Release);
        });
}

void ThreadDemoScene::DrawJsParallelForPanel() {
    ImGui::TextColored({1.0f, 0.85f, 0.2f, 1.0f}, "ParallelFor Benchmark  (M4)");
    ImGui::Spacing();

    bool running = mJsBenchmarkRunning.Load(Umbra::EMemoryOrder::Acquire);

    static int sCount = 1000;
    ImGui::SliderInt("Item count", &sCount, 100, 10000);

    if (running) {
        ImGui::BeginDisabled();
        ImGui::Button("Running...", {130.0f, 28.0f});
        ImGui::EndDisabled();
    } else {
        if (ImGui::Button("Dispatch", {90.0f, 28.0f})) {
            LaunchJsBenchmark(static_cast<Umbra::uint32>(sCount));
        }
    }

    ImGui::Spacing();

    if (running) {
        ImGui::TextColored({1.0f, 0.85f, 0.2f, 1.0f}, "  Executing...");
    } else if (mJsBenchmarkMs >= 0.0f) {
        ImGui::TextColored({0.3f, 1.0f, 0.3f, 1.0f},
            "Completed in %.3f ms", mJsBenchmarkMs);
        ImGui::Spacing();
        ImGui::TextColored({0.7f, 0.9f, 1.0f, 1.0f},
            "Preview (first 6 of %d):", (int)mJsBenchmarkResults.size());
        Umbra::uint32 n = std::min(6u, (Umbra::uint32)mJsBenchmarkResults.size());
        for (Umbra::uint32 i = 0; i < n; ++i) {
            ImGui::Text("  [%u]=%u", i, mJsBenchmarkResults[i]);
            if (i < n - 1) ImGui::SameLine(0.0f, 10.0f);
        }
    } else {
        ImGui::TextColored({0.55f, 0.55f, 0.55f, 1.0f}, "Press Dispatch to run.");
    }

    ImGui::Spacing();
    ImGui::TextColored({0.8f, 0.8f, 0.8f, 1.0f}, "How It Works:");
    ImGui::TextWrapped(
        "ParallelFor(N, fn) sets a shared AtomicInt32 to N, pushes N independent "
        "jobs under one lock, then NotifyAll() wakes every idle worker. Each job "
        "calls fn(i) and decrements the counter. The JobHandle is complete when "
        "the counter reaches zero — the callback records the wall-clock time.");
}

// ---------------------------------------------------------------------------
// Job System — Dependency chain panel (A -> B -> C)
// ---------------------------------------------------------------------------

static void JsDrawChainNode(ImDrawList* _dl, ImVec2 _pos, float _w, float _h,
                            const char* _label, float _tsMs, bool _done, double _t) {
    ImVec4 col = _done
        ? ImVec4{0.25f, 0.85f, 0.45f, 1.0f}
        : ImVec4{0.28f, 0.28f, 0.28f, 1.0f};

    _dl->AddRectFilled(_pos, {_pos.x + _w, _pos.y + _h},
        ImGui::ColorConvertFloat4ToU32(col), 6.0f);
    _dl->AddRect(_pos, {_pos.x + _w, _pos.y + _h},
        IM_COL32(100, 100, 100, 180), 6.0f, 0, 1.5f);

    if (_done) {
        float pulse = static_cast<float>(0.5 + 0.5 * sin(_t * 2.5));
        ImVec4 glow = col;
        glow.w = 0.35f * pulse;
        _dl->AddRect({_pos.x - 3.0f, _pos.y - 3.0f},
                     {_pos.x + _w + 3.0f, _pos.y + _h + 3.0f},
                     ImGui::ColorConvertFloat4ToU32(glow), 6.0f, 0, 3.0f);
    }

    // Name label
    ImVec2 ts = ImGui::CalcTextSize(_label);
    _dl->AddText({_pos.x + (_w - ts.x) * 0.5f, _pos.y + (_h - ts.y) * 0.5f - 8.0f},
                 IM_COL32(255, 255, 255, 255), _label);

    // Timestamp label
    char buf[32];
    if (_tsMs < 0.0f) {
        snprintf(buf, sizeof(buf), "pending");
    } else {
        snprintf(buf, sizeof(buf), "+%.0f ms", _tsMs);
    }
    ImVec2 ss = ImGui::CalcTextSize(buf);
    _dl->AddText({_pos.x + (_w - ss.x) * 0.5f, _pos.y + (_h - ss.y) * 0.5f + 6.0f},
                 _done ? IM_COL32(180, 255, 180, 220) : IM_COL32(140, 140, 140, 180), buf);
}

static void JsDrawArrow(ImDrawList* _dl, ImVec2 _from, ImVec2 _to) {
    _dl->AddLine(_from, _to, IM_COL32(150, 150, 150, 200), 1.5f);
    float dx  = _to.x - _from.x;
    float dy  = _to.y - _from.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    dx /= len; dy /= len;
    float ax = dx * 8.0f, ay = dy * 8.0f;
    _dl->AddTriangleFilled(
        _to,
        {_to.x - ax - ay * 0.5f, _to.y - ay + ax * 0.5f},
        {_to.x - ax + ay * 0.5f, _to.y - ay - ax * 0.5f},
        IM_COL32(150, 150, 150, 200));
}

void ThreadDemoScene::LaunchJsChain() {
    if (mJsChainRunning.Exchange(true)) return;

    mJsTsA.Store(-1.0f);
    mJsTsB.Store(-1.0f);
    mJsTsC.Store(-1.0f);
    mJsChainLaunchTime = ImGui::GetTime();
    double launch      = mJsChainLaunchTime;

    Umbra::JobSystem* js = Umbra::ServiceLocator::GetJobSystem();

    // A — 600 ms: long enough to see the node glow before B starts
    Umbra::JobHandle hA = js->Submit([this, launch]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        mJsTsA.Store(static_cast<float>((ImGui::GetTime() - launch) * 1000.0));
    });

    // B — depends on A, 800 ms: clearly starts only after A's bar goes green
    Umbra::JobHandle hB = js->SubmitAfter(hA, [this, launch]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        mJsTsB.Store(static_cast<float>((ImGui::GetTime() - launch) * 1000.0));
    });

    // C — depends on B, 400 ms; clears the running flag when done
    js->SubmitAfter(hB, [this, launch]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        mJsTsC.Store(static_cast<float>((ImGui::GetTime() - launch) * 1000.0));
        mJsChainRunning.Store(false, Umbra::EMemoryOrder::Release);
    });
}

void ThreadDemoScene::DrawJsDependencyChainPanel() {
    ImGui::TextColored({1.0f, 0.85f, 0.2f, 1.0f}, "Dependency Chain  A -> B -> C  (M3)");
    ImGui::Spacing();

    bool running = mJsChainRunning.Load(Umbra::EMemoryOrder::Acquire);

    if (running) {
        ImGui::BeginDisabled();
        ImGui::Button("Running...", {120.0f, 28.0f});
        ImGui::EndDisabled();
    } else {
        if (ImGui::Button("Run Chain", {100.0f, 28.0f})) {
            LaunchJsChain();
        }
    }

    ImGui::Spacing();

    // Node graph
    float avail        = ImGui::GetContentRegionAvail().x;
    const float kW     = 100.0f;
    const float kH     = 58.0f;
    float spacing      = std::max((avail - kW * 3.0f) * 0.25f, 14.0f);
    ImVec2 cursor      = ImGui::GetCursorScreenPos();
    ImDrawList* dl     = ImGui::GetWindowDrawList();
    double t           = ImGui::GetTime();
    float cy           = cursor.y + 8.0f;

    float tsA = mJsTsA.Load(Umbra::EMemoryOrder::Acquire);
    float tsB = mJsTsB.Load(Umbra::EMemoryOrder::Acquire);
    float tsC = mJsTsC.Load(Umbra::EMemoryOrder::Acquire);

    ImVec2 posA = {cursor.x + spacing,                      cy};
    ImVec2 posB = {cursor.x + spacing * 2.0f + kW,         cy};
    ImVec2 posC = {cursor.x + spacing * 3.0f + kW * 2.0f, cy};

    JsDrawChainNode(dl, posA, kW, kH, "Job A", tsA, tsA >= 0.0f, t);
    JsDrawChainNode(dl, posB, kW, kH, "Job B", tsB, tsB >= 0.0f, t);
    JsDrawChainNode(dl, posC, kW, kH, "Job C", tsC, tsC >= 0.0f, t);

    JsDrawArrow(dl, {posA.x + kW, cy + kH * 0.5f}, {posB.x, cy + kH * 0.5f});
    JsDrawArrow(dl, {posB.x + kW, cy + kH * 0.5f}, {posC.x, cy + kH * 0.5f});

    ImGui::Dummy({avail, kH + 18.0f});

    if (tsA >= 0.0f && tsB >= 0.0f && tsC >= 0.0f) {
        ImGui::TextColored({0.7f, 0.9f, 1.0f, 1.0f}, "Result:");
        ImGui::Text("  A: +%.0f ms", tsA);
        ImGui::Text("  B: +%.0f ms  (gap %.0f ms)", tsB, tsB - tsA);
        ImGui::Text("  C: +%.0f ms  (gap %.0f ms)", tsC, tsC - tsB);
        if ((tsA < tsB) && (tsB < tsC)) {
            ImGui::TextColored({0.3f, 1.0f, 0.3f, 1.0f}, "  A < B < C   ordering confirmed");
        } else {
            ImGui::TextColored({1.0f, 0.3f, 0.3f, 1.0f}, "  Ordering violated!");
        }
    }

    ImGui::Spacing();
    ImGui::TextColored({0.8f, 0.8f, 0.8f, 1.0f}, "How It Works:");
    ImGui::TextWrapped(
        "SubmitAfter(parent, task) appends an enqueue-callback to the parent's "
        "completion list under the callback mutex. When the parent's worker calls "
        "Decrement() and the count reaches zero, it fires all callbacks — "
        "enqueuing each child. The mutex prevents a race: if the parent completes "
        "between the 'is it done?' check and AddCallback(), the fast path detects "
        "count==0 and enqueues the child immediately.");
}

// ---------------------------------------------------------------------------
// Job System — Throughput graph panel
// ---------------------------------------------------------------------------

void ThreadDemoScene::UpdateJsThroughput(double _now) {
    Umbra::JobSystem* js = Umbra::ServiceLocator::GetJobSystem();
    if (!js) return;

    if (mJsLastSampleTime < 0.0) {
        mJsLastSampleTime = _now;
        mJsLastJobCount   = js->GetTotalJobsCompleted();
        return;
    }

    double elapsed = _now - mJsLastSampleTime;
    if (elapsed < 0.1) return;

    Umbra::uint64 current = js->GetTotalJobsCompleted();
    Umbra::uint64 delta   = current - mJsLastJobCount;
    float rate            = static_cast<float>(delta) / static_cast<float>(elapsed);

    mJsThroughputHistory[mJsGraphOffset] = rate;
    mJsGraphOffset = (mJsGraphOffset + 1) % JS_GRAPH_SAMPLES;
    if (rate > mJsPeakThroughput) mJsPeakThroughput = rate;

    mJsLastJobCount   = current;
    mJsLastSampleTime = _now;
}

void ThreadDemoScene::DrawJsThroughputPanel() {
    ImGui::TextColored({1.0f, 0.85f, 0.2f, 1.0f}, "Job Throughput  (M2 — Atomic Counter)");
    ImGui::Spacing();

    Umbra::JobSystem* js = Umbra::ServiceLocator::GetJobSystem();
    if (!js) return;

    ImGui::Text("Total jobs completed: %llu",
        (unsigned long long) js->GetTotalJobsCompleted());
    ImGui::Spacing();

    if (ImGui::Button("Burst 200 Jobs", {140.0f, 28.0f})) {
        for (int i = 0; i < 200; ++i) {
            js->Submit([]() {
                // ~1 M iterations ≈ 2–5 ms per job on modern hardware.
                // 200 jobs across N workers keeps all bars green for ~30+ frames.
                volatile int x = 0;
                for (int k = 0; k < 1000000; ++k) x += k;
                (void)x;
            });
        }
    }
    ImGui::SameLine();
    ImGui::TextDisabled("spikes the graph");
    ImGui::Spacing();

    ImGui::TextColored({0.7f, 0.9f, 1.0f, 1.0f}, "Jobs / second  (last %.0f s)",
        JS_GRAPH_SAMPLES * 0.1);

    char overlay[32];
    snprintf(overlay, sizeof(overlay), "peak: %.0f/s", mJsPeakThroughput);

    float avail = ImGui::GetContentRegionAvail().x;
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4{0.35f, 0.85f, 0.35f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_FrameBg,   ImVec4{0.10f, 0.10f, 0.10f, 1.0f});
    ImGui::PlotLines("##jsthroughput",
        mJsThroughputHistory, JS_GRAPH_SAMPLES,
        mJsGraphOffset, overlay,
        0.0f, mJsPeakThroughput > 0.0f ? mJsPeakThroughput * 1.1f : 1.0f,
        {avail, 80.0f});
    ImGui::PopStyleColor(2);

    ImGui::Spacing();
    ImGui::TextColored({0.8f, 0.8f, 0.8f, 1.0f}, "How It Works:");
    ImGui::TextWrapped(
        "Each worker increments mTotalJobsCompleted (AtomicUInt64, Relaxed) "
        "after executing each job. The main thread snapshots this counter every "
        "100 ms, divides the delta by elapsed time to get jobs/second, and "
        "appends it to a circular float buffer that PlotLines renders as a "
        "rolling graph. Relaxed ordering is sufficient — we only need atomicity, "
        "not cross-thread data synchronisation, for a display counter.");
}
