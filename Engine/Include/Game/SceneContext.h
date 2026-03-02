#pragma once
#include "Diag/Logger.h"
#include "Umbra.h"
#include <any>

namespace Umbra {

    /**
     * Arbitrary key-value bag passed between scenes during a GoToScene transition.
     *
     * Usage — sending scene:
     *   SceneContext ctx;
     *   ctx.Set("Score", 1500);
     *   ctx.Set("PlayerName", String("Alice"));
     *   ctx.Set("SpawnPos", Math::Vector2f(10.f, 0.f));
     *   GetSceneManager().GoToScene("GameOver", ctx);
     *
     * Usage — receiving scene (OnBeginPlay):
     *   const SceneContext& ctx = GetSceneContext();
     *   int score = ctx.GetOr<int>("Score", 0);
     *   String name = ctx.GetOr<String>("PlayerName", String("Player"));
     */
    class SceneContext {
    public:
        // Store a value under _key. Overwrites any existing entry.
        template <typename T>
        void Set(const String& _key, T _value) {
            mData[_key] = std::move(_value);
        }

        // Retrieve a value by key. Logs a warning and returns T{} if missing or wrong type.
        template <typename T>
        T Get(const String& _key) const {
            auto it = mData.find(_key);
            if (it == mData.end()) {
                UMBRA_LOG_WARN("SceneContext::Get — key '%s' not found", _key.c_str());
                return T{};
            }
            const T* val = std::any_cast<T>(&it->second);
            if (!val) {
                UMBRA_LOG_WARN("SceneContext::Get — key '%s' has wrong type", _key.c_str());
                return T{};
            }
            return *val;
        }

        // Retrieve a value by key, returning _default if missing or wrong type.
        template <typename T>
        T GetOr(const String& _key, T _default) const {
            auto it = mData.find(_key);
            if (it == mData.end()) return _default;
            const T* val = std::any_cast<T>(&it->second);
            return val ? *val : _default;
        }

        // Returns true if _key is present.
        bool Has(const String& _key) const {
            return mData.count(_key) > 0;
        }

        // Remove all entries.
        void Clear() {
            mData.clear();
        }

    private:
        UMap<String, std::any> mData;
    };

} // namespace Umbra
