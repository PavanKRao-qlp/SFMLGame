#pragma once
#include "ECS/Component.h"
#include "ECS/ECSConfig.h"
#include "ECS/ECSRegister.h"
#include "ECS/Enity.h"
#include "ECS/View.h"
#include "EnginePCH.h"
namespace Umbra {
    enum class ESystemPhase {
        FrameStart, // Input collection, etc.
        Simulation, // Physics, AI
        PreRender, // Visibility culling, quad submission
        FrameEnd // Cleanup
    };

    class System {
    private:
        /* data */
    public:
        System(UniquePtr<BaseView> view) {
            mView           = std::move(view);
            SystemSignature = mView->GetSignature();
        }
        // ~System();
        inline virtual void AddEntity(EntityID _entity) {
            mView->AddEntity(_entity);
        }
        inline void Flush() {
            mView->Flush();
        }
        inline virtual void RemoveEntity(EntityID _entity) {
            mView->RemoveEntity(_entity);
        }
        inline void AssignRegistry(ECSRegister* _register) {
            mView->AssignRegistry(_register);
        }
        inline void SetEnabled(bool _bShouldEnable) {
            bEnabled = _bShouldEnable;
        }
        inline bool GetEnabled() const {
            return bEnabled;
        }
        inline void SetPriority(int _priority) {
            mPriority = _priority;
        }
        inline int GetPriority() const {
            return mPriority;
        }
        virtual void Update() = 0;
        ComponentMask SystemSignature;

    protected:
        UniquePtr<BaseView> mView;
        bool bEnabled       = true;
        ESystemPhase mPhase = ESystemPhase::Simulation;
        int mPriority       = 0;
    };

    class SystemManager {
    public:
        /** Registers a system with the given phase, priority and registry context. */
        void AddSystem(ESystemPhase _phase, int _priority, SharedPtr<System> _system, ECSRegister* _registry);

        /** Removes a system from whichever phase it belongs to. */
        void RemoveSystem(SharedPtr<System>& _system);

        /** Returns the first system of type T across all phases, or nullptr. */
        template <typename T>
        SharedPtr<T> GetSystem();

        /** Enables or disables the first system of type T found across all phases. */
        template <typename T>
        void SetSystemEnabled(bool _bEnabled);

        /** Runs all enabled systems in the given phase. */
        void UpdatePhase(ESystemPhase _phase);

        /** Runs all phases in declaration order (FrameStart → Simulation → PreRender → FrameEnd). */
        void UpdateAll();

        // ── Entity lifecycle notifications (called by ECSRegister::CleanUp) ──

        /** Adds _entity to every system whose signature matches _entitySignature. */
        void OnEntityAdded(EntityID _entity, const ComponentMask& _entitySignature);

        /** Removes _entity from every registered system. */
        void OnEntityRemoved(EntityID _entity);

        /** Re-evaluates _entity against every system: adds if matching, removes otherwise. */
        void OnEntityModified(EntityID _entity, const ComponentMask& _entitySignature);

    private:
        UMap<ESystemPhase, Vector<SharedPtr<System>>> mSystemMap;
    };

    // ──────────────────────────────────────────────────────────────────────────
    // SystemManager — inline implementations
    // ──────────────────────────────────────────────────────────────────────────

    inline void SystemManager::AddSystem(
        ESystemPhase _phase, int _priority, SharedPtr<System> _system, ECSRegister* _registry) {
        if (mSystemMap.find(_phase) == mSystemMap.end()) {
            mSystemMap.emplace(_phase, Vector<SharedPtr<System>>());
        }
        _system->SetPriority(_priority);
        mSystemMap[_phase].emplace_back(_system);
        _system->AssignRegistry(_registry);

        std::stable_sort(mSystemMap[_phase].begin(), mSystemMap[_phase].end(),
            [](const SharedPtr<System>& _a, const SharedPtr<System>& _b) {
                return _a->GetPriority() < _b->GetPriority();
            });
    }

    inline void SystemManager::RemoveSystem(SharedPtr<System>& _system) {
        for (auto& pair : mSystemMap) {
            auto& vec = pair.second;
            auto it   = std::find(vec.begin(), vec.end(), _system);
            if (it != vec.end()) {
                vec.erase(it);
                return;
            }
        }
    }

    template <typename T>
    inline SharedPtr<T> SystemManager::GetSystem() {
        for (auto& pair : mSystemMap) {
            for (auto& system : pair.second) {
                SharedPtr<T> cast = std::dynamic_pointer_cast<T>(system);
                if (cast) {
                    return cast;
                }
            }
        }
        return nullptr;
    }

    template <typename T>
    inline void SystemManager::SetSystemEnabled(bool _bEnabled) {
        SharedPtr<T> system = GetSystem<T>();
        if (system) {
            system->SetEnabled(_bEnabled);
        }
    }

    inline void SystemManager::UpdatePhase(ESystemPhase _phase) {
        auto it = mSystemMap.find(_phase);
        if (it == mSystemMap.end()) {
            return;
        }
        for (const SharedPtr<System>& system : it->second) {
            if (system->GetEnabled()) {
                system->Update();
            }
        }
    }

    inline void SystemManager::UpdateAll() {
        static const ESystemPhase phases[] = {
            ESystemPhase::FrameStart,
            ESystemPhase::Simulation,
            ESystemPhase::PreRender,
            ESystemPhase::FrameEnd,
        };
        for (ESystemPhase phase : phases) {
            UpdatePhase(phase);
        }
    }

    inline void SystemManager::OnEntityAdded(EntityID _entity, const ComponentMask& _entitySignature) {
        for (auto& pair : mSystemMap) {
            for (const SharedPtr<System>& system : pair.second) {
                if ((system->SystemSignature & _entitySignature) == system->SystemSignature) {
                    system->AddEntity(_entity);
                }
            }
        }
    }

    inline void SystemManager::OnEntityRemoved(EntityID _entity) {
        for (auto& pair : mSystemMap) {
            for (const SharedPtr<System>& system : pair.second) {
                system->RemoveEntity(_entity);
            }
        }
    }

    inline void SystemManager::OnEntityModified(EntityID _entity, const ComponentMask& _entitySignature) {
        for (auto& pair : mSystemMap) {
            for (const SharedPtr<System>& system : pair.second) {
                if ((system->SystemSignature & _entitySignature) == system->SystemSignature) {
                    system->AddEntity(_entity);
                } else {
                    system->RemoveEntity(_entity);
                }
            }
        }
    }

} // namespace Umbra
