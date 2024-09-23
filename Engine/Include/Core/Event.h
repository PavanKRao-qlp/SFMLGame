#pragma once
#include "EnginePCH.h"
namespace UMBRA
{

#define BIND(obj, func) std::bind(func, obj)
#define BIND_1P(obj, func) std::bind(func, obj, std::placeholders::_1)
#define BIND_2P(obj, func) std::bind(func, obj, std::placeholders::_1, std::placeholders::_2);
#define BIND_3P(obj, func) std::bind(func, obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
#define FUNC(retType, ...) std::function<retType(__VA_ARGS__)>

    template <typename retType, typename... args>
    class Delegate
    {
        using delegateCallback = std::function<retType(args...)>;

    public:
        inline void AddToInvocationList(const delegateCallback &callback)
        {
            mCallBack = callback;
        };

        inline retType Broadcast(args... inArgs)
        {
            return mCallBack(inArgs...);
        };

    private:
        delegateCallback mCallBack;
    };

    class EventBus
    {
    public:
        inline EventBus() {};

        inline static void Flush()
        {
            for (auto events : EventBus::CallbackMap)
            {
                delete events.second;
            }
        }

        template <typename T>
        inline static void Subscribe(FUNC(void, const T &) callback)
        {
            using EventDelegate = Delegate<void, T>;
            int64 eventType = typeid(T).hash_code();
            if (EventBus::CallbackMap.find(eventType) != EventBus::CallbackMap.end())
            {
                EventDelegate *delegate = CAST(EventDelegate *, CallbackMap[eventType]);
                delegate->AddToInvocationList(callback);
            }
            else
            {
                EventDelegate *delegate = new EventDelegate();
                delegate->AddToInvocationList(callback);
                void *v = CAST(void *, delegate);
                EventBus::CallbackMap[eventType] = v;
            }
        };

        template <typename T>
        inline static void FireEvent(T *Event)
        {
            using EventDelegate = Delegate<void, T>;
            int64 eventType = typeid(T).hash_code();
            if (EventBus::CallbackMap.find(eventType) != EventBus::CallbackMap.end())
            {
                auto delegate = CAST(EventDelegate *, CallbackMap[eventType]);
                delegate->Broadcast(*Event);
            }
            else
            {
                EventDelegate *delegate = new EventDelegate();
                void *v = CAST(void *, delegate);
                EventBus::CallbackMap[eventType] = v;
            }
        };

        static inline std::map<int64, void *> CallbackMap;
    };

    class Event
    {
    };
}