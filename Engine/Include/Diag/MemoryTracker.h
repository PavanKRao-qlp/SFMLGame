#pragma once
#include "EnginePCH.h"
#include <atomic>

namespace Umbra {

    // ─── Memory categories ────────────────────────────────────────────────────
    enum class EMemoryCategory : uint8 {
        General = 0,
        ECS,
        Physics,
        Asset,
        Audio,
        UI,
        COUNT
    };

    // ─── RAII scope guard ─────────────────────────────────────────────────────
    // Tags all allocations on this thread to _category until the scope exits.
    // Nestable — restores the previous category on destruction.
    //
    // Usage:
    //   {
    //       UMBRA_ALLOC_SCOPE(EMemoryCategory::ECS);
    //       auto p = std::make_shared<MyComponent>(); // tracked as ECS
    //   }
    struct ScopedAllocCategory {
        explicit ScopedAllocCategory(EMemoryCategory _category);
        ~ScopedAllocCategory();
    private:
        EMemoryCategory mPrev;
    };

    // ─── MemoryTracker ────────────────────────────────────────────────────────
    // Receives callbacks from the global operator new/delete overrides.
    // All state is plain std::atomic<int64_t> — zero-initialised before main(),
    // no heap allocation needed, no singleton bootstrapping hazard.
    class MemoryTracker {
    public:
        struct CategoryStats {
            std::atomic<int64_t> LiveBytes{0};
            std::atomic<int64_t> AllocCount{0};
        };

        // Called by global operator new/delete — not for manual use.
        static void OnAlloc(size_t _size, EMemoryCategory _cat);
        static void OnFree(size_t _size, EMemoryCategory _cat);

        // ─── Query API ────────────────────────────────────────────────────────
        static int64_t              GetTotalLiveBytes();
        static int64_t              GetTotalAllocCount();
        static const CategoryStats& GetCategoryStats(EMemoryCategory _cat);
        static const char*          GetCategoryName(EMemoryCategory _cat);

        // Draws an ImGui window — call between NewFrame() and Render().
        static void DrawImGuiPanel();

    private:
        static std::atomic<int64_t> sTotalLiveBytes;
        static std::atomic<int64_t> sTotalAllocCount;
        static CategoryStats        sCategoryStats[static_cast<size_t>(EMemoryCategory::COUNT)];
    };

} // namespace Umbra

// Tag all allocations on this thread to a category for the current scope.
#define UMBRA_ALLOC_SCOPE(_cat) Umbra::ScopedAllocCategory _umbraAllocScope##__LINE__(_cat)
