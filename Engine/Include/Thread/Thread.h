#pragma once
#include "Atomic.h"
#include "EnginePCH.h"
#include <thread>
#include <tuple>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
// Undefine Windows macros that expand method-call tokens to WinAPI names.
// Engine code wraps platform calls through SFML; raw CreateWindow / min / max
// macros must not pollute translation units that include this header.
#ifdef CreateWindow
#    undef CreateWindow
#endif
#ifdef CreateWindowEx
#    undef CreateWindowEx
#endif
#ifdef min
#    undef min
#endif
#ifdef max
#    undef max
#endif
#endif

namespace Umbra {

    enum class EThreadState : uint8 {
        Idle, // Created, not yet started
        Running, // Currently executing
        Finished, // Completed execution
        Detached // Detached, no longer owned by this handle
    };

    class Thread {
    public:
        Thread() = default;

        explicit Thread(const String& _name) : mName(_name) {}

        // Auto-joins on destruction (RAII)
        ~Thread() {
            if (mThread.joinable()) {
                mThread.join();
            }
        }

        Thread(const Thread&)            = delete;
        Thread& operator=(const Thread&) = delete;

        Thread(Thread&& _other) noexcept : mName(std::move(_other.mName)), mThread(std::move(_other.mThread)) {
            mState.Store(_other.mState.Exchange(EThreadState::Idle));
        }

        Thread& operator=(Thread&& _other) noexcept {
            if (this != &_other) {
                if (mThread.joinable()) {
                    mThread.join();
                }
                mName   = std::move(_other.mName);
                mThread = std::move(_other.mThread);
                mState.Store(_other.mState.Exchange(EThreadState::Idle));
            }
            return *this;
        }

        // Launch the thread with a callable and optional arguments
        template <typename Fn, typename... Args>
        void Start(Fn&& _fn, Args&&... _args) {
            mState.Store(EThreadState::Running);
            // copy name in lambda
            String capturedName = mName;
            mThread             = std::thread([this, capturedName, fn = std::forward<Fn>(_fn),
                                      capturedArgs = std::make_tuple(std::forward<Args>(_args)...)]() mutable {
                SetPlatformName(capturedName);
                // same as calling (fn (capArg(0), capArg(1), ....))
                std::apply(std::move(fn), std::move(capturedArgs));
                mState.Store(EThreadState::Finished);
            });
        }

        // Block until the thread finishes
        void Join() {
            if (mThread.joinable()) {
                mThread.join();
            }
        }

        // Release ownership — thread runs independently
        void Detach() {
            if (mThread.joinable()) {
                mThread.detach();
                mState.Store(EThreadState::Detached);
            }
        }

        bool IsJoinable() const {
            return mThread.joinable();
        }

        EThreadState GetState() const {
            return mState.Load();
        }

        const String& GetName() const {
            return mName;
        }

        std::thread::id GetID() const {
            return mThread.get_id();
        }

    private:
        // Called from within the new thread so the OS sees the name on that thread
        static void SetPlatformName(const String& _name) {
#ifdef _WIN32
            std::wstring wName(_name.begin(), _name.end());
            SetThreadDescription(GetCurrentThread(), wName.c_str());
#elif defined(__linux__)
            pthread_setname_np(pthread_self(), _name.substr(0, 15).c_str());
#elif defined(__APPLE__)
            pthread_setname_np(_name.substr(0, 63).c_str());
#endif
        }

        String mName;
        std::thread mThread;
        Atomic<EThreadState> mState{EThreadState::Idle};
    };

} // namespace Umbra
