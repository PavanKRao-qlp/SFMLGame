#include "Diag/MemoryTracker.h"

#include "imgui.h"

#include <cstdlib>
#include <cstdio>
#include <new>

// ─── Thread-local current allocation category ────────────────────────────────
// Zero-initialised by the runtime before any dynamic init — safe to read from
// inside operator new even before main() starts.
thread_local static Umbra::EMemoryCategory gCurrentAllocCategory = Umbra::EMemoryCategory::General;

// ─── Allocation header ────────────────────────────────────────────────────────
// Prepended to every heap block so that operator delete can retrieve the size
// and category without relying on the sized-delete hint from the compiler.
//
// kHeaderSize is rounded up to alignof(max_align_t) so the returned user
// pointer has the same alignment guarantee as a raw malloc() call.
namespace {

    struct AllocHeader {
        size_t  size;
        uint8_t category;
    };

    constexpr size_t kAlign      = alignof(std::max_align_t); // 16 on MSVC x64
    constexpr size_t kHeaderSize = (sizeof(AllocHeader) + kAlign - 1) & ~(kAlign - 1);

    static_assert(kHeaderSize >= sizeof(AllocHeader), "kHeaderSize too small");

    inline void* RawToUser(void* _raw) noexcept {
        return static_cast<char*>(_raw) + kHeaderSize;
    }
    inline void* UserToRaw(void* _user) noexcept {
        return static_cast<char*>(_user) - kHeaderSize;
    }
    inline AllocHeader* GetHeader(void* _user) noexcept {
        return static_cast<AllocHeader*>(UserToRaw(_user));
    }

    void* DoAlloc(size_t _size) noexcept {
        void* raw = malloc(_size + kHeaderSize);
        if (!raw) return nullptr;
        auto* h    = static_cast<AllocHeader*>(raw);
        h->size     = _size;
        h->category = static_cast<uint8_t>(gCurrentAllocCategory);
        Umbra::MemoryTracker::OnAlloc(_size, static_cast<Umbra::EMemoryCategory>(h->category));
        return RawToUser(raw);
    }

    void DoFree(void* _ptr) noexcept {
        if (!_ptr) return;
        AllocHeader* h = GetHeader(_ptr);
        Umbra::MemoryTracker::OnFree(h->size, static_cast<Umbra::EMemoryCategory>(h->category));
        free(UserToRaw(_ptr));
    }

} // anonymous namespace

// ─── Global operator new / delete overrides ───────────────────────────────────

void* operator new(std::size_t size) {
    void* ptr = DoAlloc(size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void* operator new[](std::size_t size) {
    void* ptr = DoAlloc(size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
    return DoAlloc(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
    return DoAlloc(size);
}

void operator delete(void* ptr) noexcept {
    DoFree(ptr);
}

void operator delete[](void* ptr) noexcept {
    DoFree(ptr);
}

// Sized-delete variants — size is already stored in the header, so just forward.
void operator delete(void* ptr, std::size_t) noexcept {
    DoFree(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept {
    DoFree(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept {
    DoFree(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
    DoFree(ptr);
}

// ─── MemoryTracker implementation ─────────────────────────────────────────────

namespace Umbra {

    // Static member definitions
    std::atomic<int64_t>        MemoryTracker::sTotalLiveBytes{0};
    std::atomic<int64_t>        MemoryTracker::sTotalAllocCount{0};
    MemoryTracker::CategoryStats MemoryTracker::sCategoryStats[static_cast<size_t>(EMemoryCategory::COUNT)];

    // ─── ScopedAllocCategory ─────────────────────────────────────────────────

    ScopedAllocCategory::ScopedAllocCategory(EMemoryCategory _category) {
        mPrev                 = gCurrentAllocCategory;
        gCurrentAllocCategory = _category;
    }

    ScopedAllocCategory::~ScopedAllocCategory() {
        gCurrentAllocCategory = mPrev;
    }

    // ─── Tracking callbacks ───────────────────────────────────────────────────

    void MemoryTracker::OnAlloc(size_t _size, EMemoryCategory _cat) {
        sTotalLiveBytes.fetch_add(static_cast<int64_t>(_size), std::memory_order_relaxed);
        sTotalAllocCount.fetch_add(1, std::memory_order_relaxed);
        size_t idx = static_cast<size_t>(_cat);
        sCategoryStats[idx].LiveBytes.fetch_add(static_cast<int64_t>(_size), std::memory_order_relaxed);
        sCategoryStats[idx].AllocCount.fetch_add(1, std::memory_order_relaxed);
    }

    void MemoryTracker::OnFree(size_t _size, EMemoryCategory _cat) {
        sTotalLiveBytes.fetch_sub(static_cast<int64_t>(_size), std::memory_order_relaxed);
        sTotalAllocCount.fetch_sub(1, std::memory_order_relaxed);
        size_t idx = static_cast<size_t>(_cat);
        sCategoryStats[idx].LiveBytes.fetch_sub(static_cast<int64_t>(_size), std::memory_order_relaxed);
        sCategoryStats[idx].AllocCount.fetch_sub(1, std::memory_order_relaxed);
    }

    // ─── Query API ────────────────────────────────────────────────────────────

    int64_t MemoryTracker::GetTotalLiveBytes() {
        return sTotalLiveBytes.load(std::memory_order_relaxed);
    }

    int64_t MemoryTracker::GetTotalAllocCount() {
        return sTotalAllocCount.load(std::memory_order_relaxed);
    }

    const MemoryTracker::CategoryStats& MemoryTracker::GetCategoryStats(EMemoryCategory _cat) {
        return sCategoryStats[static_cast<size_t>(_cat)];
    }

    const char* MemoryTracker::GetCategoryName(EMemoryCategory _cat) {
        switch (_cat) {
        case EMemoryCategory::General:  return "General";
        case EMemoryCategory::ECS:      return "ECS";
        case EMemoryCategory::Physics:  return "Physics";
        case EMemoryCategory::Asset:    return "Asset";
        case EMemoryCategory::Audio:    return "Audio";
        case EMemoryCategory::UI:       return "UI";
        default:                        return "Unknown";
        }
    }

    // ─── ImGui panel ─────────────────────────────────────────────────────────

    void MemoryTracker::DrawImGuiPanel() {
        ImGui::SetNextWindowSize(ImVec2(440.f, 300.f), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Memory Tracker")) {
            ImGui::End();
            return;
        }

        const int64_t totalBytes  = GetTotalLiveBytes();
        const int64_t totalAllocs = GetTotalAllocCount();

        ImGui::Text("Live:   %.2f KB  (%.2f MB)", totalBytes / 1024.0, totalBytes / (1024.0 * 1024.0));
        ImGui::Text("Allocs: %lld live", (long long)totalAllocs);
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("By Category:");
        ImGui::Spacing();

        constexpr size_t kCatCount = static_cast<size_t>(EMemoryCategory::COUNT);

        for (size_t i = 0; i < kCatCount; ++i) {
            const int64_t liveBytes  = sCategoryStats[i].LiveBytes.load(std::memory_order_relaxed);
            const int64_t liveAllocs = sCategoryStats[i].AllocCount.load(std::memory_order_relaxed);

            const float fraction = (totalBytes > 0)
                ? static_cast<float>(liveBytes) / static_cast<float>(totalBytes)
                : 0.f;

            char overlay[64];
            std::snprintf(overlay, sizeof(overlay),
                "%.1f KB  (%lld allocs)", liveBytes / 1024.0, (long long)liveAllocs);

            // Category name, left-aligned in a fixed column
            ImGui::Text("%-8s", GetCategoryName(static_cast<EMemoryCategory>(i)));
            ImGui::SameLine(90.f);

            // Progress bar fills remaining width
            ImGui::ProgressBar(fraction, ImVec2(ImGui::GetContentRegionAvail().x, 0.f), overlay);
        }

        ImGui::End();
    }

} // namespace Umbra
