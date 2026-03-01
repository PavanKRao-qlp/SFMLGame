# Render Service Design Plan

## Motivation

Currently, rendering in Umbra is tightly coupled to the ECS. The `RenderSystem` directly accesses `IRenderDevice` through `GEngineStatics`, debug drawing lives as static methods on `RenderSystem`, and there's no centralized place to manage render state, layers, or draw ordering outside of the ECS. This mirrors the state physics was in before `PhysicsService` was introduced.

A `RenderService` extracts rendering concerns into a standalone service that:
- Can be used without the ECS (tools, editors, tests)
- Provides a clean API for submitting draw commands
- Centralizes debug drawing, camera state, and layer management
- Follows the established `ServiceLocator` + handle pattern

---

## Architecture Overview

```
  Scene / User Code
        |
        v
  RenderService  <--- ServiceLocator::GetRenderService()
   |          |
   |     DebugDrawer
   v
  RenderQueue (sorted draw commands)
        |
        v
  IRenderDevice (abstraction)
        |
        v
  SfmlRenderDevice (backend)
```

**ECS Bridge**: A `RenderSyncSystem` (PreRender phase) reads `TransformComponent` + `SpriteComponent` and submits draw commands to `RenderService` each frame --- the same pattern `PhysicsSyncSystem` uses.

---

## Components

### 1. RenderService (`Engine/Include/Service/Render/RenderService.h`)

The core service class. Owns render state and provides the public API.

```cpp
namespace Umbra
{
    struct RenderStats
    {
        uint32 DrawCalls = 0;
        uint32 QuadsRendered = 0;
        uint32 QuadsCulled = 0;
        uint32 DebugPrimitives = 0;
    };

    class RenderService
    {
    public:
        void Initialize(IRenderDevice* _device);
        void Shutdown();

        // --- Camera ---
        void SetActiveCamera(const Math::Vector2f& _center, float _orthoSize);
        Math::Vector2f GetCameraCenter() const;
        float GetCameraOrthoSize() const;

        // --- Draw Submission ---
        void SubmitQuad(const RenderQuad& _quad);
        void SubmitQuads(const Vector<RenderQuad>& _quads);

        // --- Debug Drawing ---
        void DebugDrawLine(const Math::Vector2f& _from, const Math::Vector2f& _to, const Color& _color);
        void DebugDrawCircle(const Math::Vector2f& _pos, float _radius, bool _filled, const Color& _color);
        void DebugDrawBox(const Math::Bounds& _bounds, bool _filled, const Color& _color);
        void DebugDrawOrientedBox(const Math::Bounds& _bounds, float _angle, bool _filled, const Color& _color);

        // --- Frame Lifecycle ---
        void BeginFrame();       // Clear queues, reset stats
        void Flush();            // Sort, batch, and draw everything
        void EndFrame();         // Display

        // --- Stats ---
        const RenderStats& GetStats() const;

    private:
        IRenderDevice* mDevice = nullptr;
        Vector<RenderQuad> mRenderQueue;
        DebugDrawer mDebugDrawer;
        RenderStats mStats;

        // Camera state
        Math::Vector2f mCameraCenter;
        float mCameraOrthoSize = 0.f;
    };
}
```

### 2. RenderQuad (`Engine/Include/Service/Render/RenderTypes.h`)

Describes a single drawable quad submitted to the service.

```cpp
namespace Umbra
{
    struct RenderQuad
    {
        Math::Vector2f Position;
        Math::Vector2f Size;
        Math::Vector2f Pivot = {0.5f, 0.5f};
        float Angle = 0.f;
        int32 ZOrder = 0;
        int32 Layer = 0;            // Render layer (future use)
        Color Tint = Color::White;
        SharedPtr<Texture> RefTexture;
        FloatRect UVRect = {0.f, 0.f, 1.f, 1.f};
    };
}
```

### 3. DebugDrawer (`Engine/Include/Service/Render/DebugDrawer.h`)

Replaces the static debug draw methods currently on `RenderSystem`. Stores primitives and flushes them through `IRenderDevice`.

```cpp
namespace Umbra
{
    class DebugDrawer
    {
    public:
        void DrawLine(const Math::Vector2f& _from, const Math::Vector2f& _to, const Color& _color);
        void DrawCircle(const Math::Vector2f& _pos, float _radius, bool _filled, const Color& _color);
        void DrawBox(const Math::Bounds& _bounds, bool _filled, const Color& _color);
        void DrawOrientedBox(const Math::Bounds& _bounds, float _angle, bool _filled, const Color& _color);

        void Flush(IRenderDevice* _device);  // Draw all queued primitives and clear
        void Clear();

        uint32 GetPrimitiveCount() const;

    private:
        struct DebugLine { Math::Vector2f From, To; Color LineColor; };
        struct DebugCircle { Math::Vector2f Pos; float Radius; bool Filled; Color CircleColor; };
        struct DebugBox { Math::Bounds BoxBounds; float Angle; bool Filled; Color BoxColor; };

        Vector<DebugLine> mLines;
        Vector<DebugCircle> mCircles;
        Vector<DebugBox> mBoxes;
    };
}
```

### 4. ServiceLocator Update (`Engine/Include/Service/ServiceLocator.h`)

Add `RenderService` alongside `PhysicsService`:

```cpp
class ServiceLocator
{
public:
    static void Initialize();

    static PhysicsService& GetPhysicsService();
    static void RegisterPhysicsService(UniquePtr<PhysicsService> _service);

    static RenderService& GetRenderService();        // NEW
    static void RegisterRenderService(UniquePtr<RenderService> _service);  // NEW

private:
    static UniquePtr<PhysicsService> sPhysicsService;
    static UniquePtr<RenderService> sRenderService;   // NEW
};
```

### 5. RenderSyncSystem (`Engine/Include/ECS/Systems/RenderSyncSystem.h`)

Bridges ECS components to the RenderService. Registered at **PreRender** phase. Replaces the current `RenderSystem`'s entity iteration.

```cpp
namespace Umbra
{
    class RenderSyncSystem : public System
    {
    public:
        void Update(ECSRegister& _register, float _dt) override
        {
            auto& renderService = ServiceLocator::GetRenderService();

            // Sync camera
            // (CameraSystem already sets camera - or we absorb camera sync here)

            // Submit sprite quads
            auto view = _register.GetView<TransformComponent, SpriteComponent>();
            for (auto entity : view)
            {
                auto& transform = _register.GetComponent<TransformComponent>(entity);
                auto& sprite = _register.GetComponent<SpriteComponent>(entity);

                RenderQuad quad;
                quad.Position = transform.Position;
                quad.Size = transform.Size;
                quad.Pivot = transform.Pivot;
                quad.Angle = transform.Angle;
                quad.ZOrder = sprite.zOrder;
                quad.Tint = sprite.color;
                quad.RefTexture = sprite.refTexture;
                quad.UVRect = sprite.uvRect;

                renderService.SubmitQuad(quad);
            }
        }
    };
}
```

---

## Frame Lifecycle

The render frame flow changes from the current approach:

### Current Flow
```
Scene::Render()
  -> Clear screen
  -> OnUpdate() (user code, debug draws via RenderSystem statics)
  -> World::Render()
       -> FrameStart phase
       -> PreRender phase (CameraSystem)
       -> Render phase (RenderSystem: cull, sort, batch, draw, flush debug)
       -> FrameEnd phase
App::OnUpdate()
  -> ImGui NewFrame
  -> SceneManager::Render()
  -> ImGui Render
  -> Display
```

### New Flow
```
App::OnUpdate()
  -> RenderService::BeginFrame()        // clear queues, reset stats
  -> ImGui NewFrame
  -> SceneManager::Render()
       -> Scene::Render()
            -> OnUpdate()               // user code, debug draws via RenderService
            -> World::Render()
                 -> FrameStart
                 -> PreRender (CameraSystem sets camera on RenderService)
                 -> PreRender (RenderSyncSystem submits quads)
                 -> FrameEnd
  -> RenderService::Flush()             // sort, cull, batch, draw, flush debug
  -> ImGui Render
  -> RenderService::EndFrame()          // display
```

Key change: **The Render ECS phase is eliminated.** The `RenderService` handles sorting, culling, batching, and drawing in `Flush()`. The ECS only submits data during PreRender.

---

## Migration Strategy

### Phase 1: Introduce RenderService (non-breaking)

1. Create `RenderService`, `RenderQuad`, `DebugDrawer` files
2. Register in `ServiceLocator`
3. Initialize in `App::Init()` with the `IRenderDevice*`
4. Wire `BeginFrame()` / `Flush()` / `EndFrame()` into `App::OnUpdate()`
5. Move debug draw storage from `RenderSystem` statics into `DebugDrawer`
6. Keep `RenderSystem` static methods as thin forwards to `ServiceLocator::GetRenderService()`

**Result**: Existing code continues to work. Debug draws route through the service internally.

### Phase 2: RenderSyncSystem

1. Create `RenderSyncSystem`
2. Register it in `World` at PreRender phase (after CameraSystem)
3. Move frustum culling and quad submission logic from `RenderSystem::Update()` into `RenderService::Flush()`
4. Remove entity iteration from `RenderSystem` - it becomes a thin shell or is removed
5. Update `CameraSystem` to set camera on `RenderService` instead of directly on `IRenderDevice`

**Result**: ECS entities render through the service. `RenderSystem` can be deleted.

### Phase 3: Cleanup

1. Remove `RenderSystem` entirely
2. Remove the `Render` ECS phase if no other systems use it
3. Update all demo scenes that use `RenderSystem::DebugDraw*` to use `ServiceLocator::GetRenderService().DebugDraw*` (or keep static convenience wrappers)
4. Update CLAUDE.md and architecture docs

---

## File Plan

```
Engine/Include/Service/Render/
    RenderService.h
    RenderTypes.h          (RenderQuad struct)
    DebugDrawer.h

Engine/src/Service/Render/
    RenderService.cpp      (Flush logic: sort, cull, batch, draw)
    DebugDrawer.cpp

Engine/Include/ECS/Systems/
    RenderSyncSystem.h     (new, replaces RenderSystem)
```

Modified files:
- `Engine/Include/Service/ServiceLocator.h` - add RenderService
- `Engine/src/Service/ServiceLocator.cpp` - add RenderService
- `Engine/src/Core/App.cpp` - wire BeginFrame/Flush/EndFrame
- `Engine/src/Game/World.cpp` - register RenderSyncSystem, remove RenderSystem
- `Engine/Include/ECS/Systems/RenderSystem.h` - forward debug draws, then delete

---

## Design Decisions

| Decision | Rationale |
|----------|-----------|
| No handles for renderables | Unlike physics bodies that persist, quads are submitted every frame. Handles add complexity with no benefit for fire-and-forget draws. |
| Camera state on service | Frustum culling happens in `Flush()` which needs camera bounds. Keeping camera state on the service avoids reaching back into ECS. |
| DebugDrawer as separate class | Keeps RenderService focused. DebugDrawer can be reused or extended independently (e.g., persistent debug draws, categories, toggling). |
| Eliminate Render ECS phase | Rendering isn't really an ECS concern - it's output. Systems should only prepare data (PreRender). The service handles actual drawing. |
| Static convenience wrappers | User code like `RenderSystem::DebugDrawCircle()` is ergonomic. We can keep thin static wrappers that forward to the service to avoid breaking every scene file. |

---

## Future Extensions

Once the RenderService exists, these become straightforward additions:

- **Render Layers**: Filter and sort by `Layer` field, allow layer-level visibility toggling
- **Render Targets**: Off-screen rendering for post-processing, minimap, etc.
- **Text Rendering**: Submit text draw commands alongside quads
- **Particle Rendering**: Batch particle quads efficiently
- **Render Stats Overlay**: ImGui panel showing DrawCalls, QuadsRendered, etc.
- **Sprite Atlasing**: Service-level texture atlas management
