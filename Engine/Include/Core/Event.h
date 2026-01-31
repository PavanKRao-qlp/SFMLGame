#pragma once
#include "Core/Singleton.h"
#include "EnginePCH.h"
namespace Umbra {

#define BIND(obj, func)    std::bind(func, obj)
#define BIND_1P(obj, func) std::bind(func, obj, std::placeholders::_1)
#define BIND_2P(obj, func) std::bind(func, obj, std::placeholders::_1, std::placeholders::_2);
#define BIND_3P(obj, func) std::bind(func, obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
#define FUNC(retType, ...) std::function<retType(__VA_ARGS__)>

    template <typename retType, typename... args>
    class Delegate {
        using delegateCallback = std::function<retType(args...)>;

    public:
        using CallbackHandle = size_t;

        inline CallbackHandle AddToInvocationList(const delegateCallback& callback) {
            CallbackHandle handle = mNextHandle++;
            mCallbacks.push_back({handle, callback});
            return handle;
        }

        inline bool RemoveFromInvocationList(CallbackHandle _handle) {
            auto it = std::find_if(mCallbacks.begin(), mCallbacks.end(),
                                   [_handle](const auto& pair) { return pair.first == _handle; });
            if (it != mCallbacks.end()) {
                mCallbacks.erase(it);
                return true;
            }
            return false;
        }

        inline void Broadcast(args... inArgs) {
            for (auto& callbackPair : mCallbacks) {
                callbackPair.second(inArgs...);
            }
        }

    private:
        Vector<std::pair<CallbackHandle, delegateCallback>> mCallbacks;
        CallbackHandle mNextHandle = 0;
    };

    class EventBus : public Singleton<EventBus> {
    public:
        using CallbackHandle = size_t;

        static inline void Initialize() {}
        inline EventBus() {}
        inline ~EventBus() {
            Flush();
        }

        inline void Flush() {
            for (auto& events : CallbackMap) {
                delete events.second;
            }
            CallbackMap.clear();
        }

        template <typename T>
        inline static CallbackHandle Subscribe(FUNC(void, const T&) callback) {
            using EventDelegate = Delegate<void, const T&>;
            int64 eventType     = typeid(T).hash_code();
            EventDelegate* delegate = nullptr;

            auto it = GetInstance()->CallbackMap.find(eventType);
            if (it != GetInstance()->CallbackMap.end()) {
                delegate = CAST(EventDelegate*, it->second);
            } else {
                delegate = new EventDelegate();
                GetInstance()->CallbackMap[eventType] = CAST(void*, delegate);
            }

            return delegate->AddToInvocationList(callback);
        }

        template <typename T>
        inline static bool Unsubscribe(CallbackHandle _handle) {
            using EventDelegate = Delegate<void, const T&>;
            int64 eventType     = typeid(T).hash_code();

            auto it = GetInstance()->CallbackMap.find(eventType);
            if (it != GetInstance()->CallbackMap.end()) {
                EventDelegate* delegate = CAST(EventDelegate*, it->second);
                return delegate->RemoveFromInvocationList(_handle);
            }
            return false;
        }

        template <typename T>
        inline static void FireEvent(const T& _event) {
            using EventDelegate = Delegate<void, const T&>;
            int64 eventType     = typeid(T).hash_code();

            auto it = GetInstance()->CallbackMap.find(eventType);
            if (it != GetInstance()->CallbackMap.end()) {
                auto delegate = CAST(EventDelegate*, it->second);
                delegate->Broadcast(_event);
            }
        }

    protected:
        std::map<int64, void*> CallbackMap;
    };

    class Event {};
} // namespace Umbra
