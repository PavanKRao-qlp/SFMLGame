# UMBRA Game Engine

A 2D game engine built on SFML, using an Entity Component System (ECS) architecture with a scene-based workflow. Written in C++17 with CMake.

---

## Table of Contents

- [Architecture Overview](#architecture-overview)
- [ECS System](#ecs-system)
- [Core Components](#core-components)
- [Built-in Systems](#built-in-systems)
- [Key APIs](#key-apis)
- [Creating a New Project](#creating-a-new-project)
- [Building](#building)
- [Project Structure](#project-structure)
- [Dependencies](#dependencies)

---

## Architecture Overview

```
App
 └── IGameInstance          ← your entry point
      └── SceneManager
           └── Scene        ← one active at a time
                └── World
                     ├── ECSRegister   (entities + components)
                     ├── SystemManager (ordered system execution)
                     ├── PhysicsService
                     └── AudioService
```

**Execution loop phases** (run in order each frame):

| Phase | Priority convention | Purpose |
|---|---|---|
| `FrameStart` | — | Input collection |
| `Simulation` | lower = first | Physics, AI, animation |
| `PreRender` | lower = first | Scene graph, visibility |
| `FrameEnd` | — | Cleanup, lifetime |

The `Render` pass runs outside the phase system (called directly by `Scene::Render()`).

---

## ECS System

### Entities

```cpp
EntityID entity = world->CreateEntity();
world->DestroyEntity(entity);
```

`EntityID` is a plain integer. `MAX_ENTITY` is the sentinel "no entity" value.

### Components

Add, remove, query components using the `World` facade:

```cpp
world->AddComponent<TransformComponent>(entity, position, size, angle);
world->AddComponent<SpriteComponent>(entity, texture, Color::White);

bool has   = world->HasComponent<TransformComponent>(entity);
auto* comp = world->GetComponent<TransformComponent>(entity);
world->RemoveComponent<TransformComponent>(entity);
```

### Custom Systems

```cpp
class MySystem : public Umbra::System {
public:
    MySystem()
        : System(std::make_unique<ECView<TransformComponent, SpriteComponent>>()) {}

    void Update() override {
        for (EntityID e : mView->mEntities) {
            auto* t = mView->ecsRegister->GetComponent<TransformComponent>(e);
            // ... logic ...
        }
    }
};

// Register in Scene::Initialize():
auto sys = std::make_shared<MySystem>();
world->AddSystem(ESystemPhase::Simulation, /*priority=*/10, sys);
```

Lower priority values run first within a phase.

---

## Core Components

### `TransformComponent`

Carries local TRS, world-space result (written by `SceneGraphSystem`), and scene hierarchy links.

```cpp
struct TransformComponent {
    // Local (relative to parent)
    Math::Vector2f Position;
    float          Angle;   // degrees
    Math::Vector2f Scale;

    // Shape (not inherited)
    Math::Vector2f Size;
    Math::Vector2f Pivot;   // 0..1, default (0.5, 0.5)

    // World (read-only, written by SceneGraphSystem each PreRender)
    Math::Vector2f   WorldPosition;
    float            WorldAngle;
    Math::Vector2f   WorldScale;
    Math::Matrix3x3f WorldMatrix;

    // Hierarchy
    EntityID         Parent;    // MAX_ENTITY = root
    Vector<EntityID> Children;

    Math::Vector2f GetForward() const;
    Math::Vector2f GetUp()      const;
};
```

Hierarchy API (on `World`):
```cpp
world->SetParent(child, parent);
world->DetachFromParent(child);
EntityID parent   = world->GetParent(entity);
auto& children    = world->GetChildren(entity);
```

> Physics entities must be roots — do not parent them.

---

### `SpriteComponent`

```cpp
struct SpriteComponent {
    Color              color;
    SharedPtr<Texture> refTexture;
    int32              zOrder = 0;
    FloatRect          uvRect = {0, 0, 1, 1}; // normalized UVs
};
```

---

### `RigidbodyHandleComponent`

Bridges an entity to the `PhysicsService`.

```cpp
struct RigidbodyHandleComponent {
    // Configuration (set before adding component, or change and re-add)
    float Mass              = 1.0f;
    float Inertia           = 0.0f;  // 0 = auto-calculate
    float LinearDamping     = 0.0f;
    float AngularDamping    = 0.0f;
    float CoefOfRestitution = 1.0f;  // 0 = inelastic, 1 = elastic
    float StaticFriction    = 0.6f;
    float DynamicFriction   = 0.4f;
    bool  bAffectedByGravity = true;
    bool  bIsKinematic       = false; // moved by game code, not physics solver
    bool  bCanSleep          = true;  // allow sleeping when stationary
    bool  bEnableCCD         = false; // continuous collision detection (fast bodies)
    CollisionFilter Filter;           // bitmask: CategoryBits / MaskBits
};
```

Physics forces via `World`:
```cpp
world->ApplyForce(entity, {100.f, 0.f});
world->ApplyImpulse(entity, {0.f, 500.f});
world->SetVelocity(entity, {5.f, 0.f});
Math::Vector2f vel = world->GetVelocity(entity);
```

---

### Colliders

```cpp
// Axis-aligned box
world->AddComponent<BoxColliderComponent>(entity, Math::Vector2f{32.f, 32.f});

// Circle
world->AddComponent<CircleColliderComponent>(entity, /*radius=*/16.f);
```

Both inherit `ColliderComponent` which has `Offset` and `bIsTrigger`.

---

### `CollisionCallbackComponent`

```cpp
struct CollisionCallbackComponent {
    std::function<void(EntityID, EntityID, const CollisionEvent&)> OnCollisionEnter;
    std::function<void(EntityID, EntityID, const CollisionEvent&)> OnCollisionStay;
    std::function<void(EntityID, EntityID)>                        OnCollisionExit;

    std::function<void(EntityID, EntityID, const CollisionEvent&)> OnTriggerEnter;
    std::function<void(EntityID, EntityID, const CollisionEvent&)> OnTriggerStay;
    std::function<void(EntityID, EntityID)>                        OnTriggerExit;
};
```

---

### `AnimatorComponent`

```cpp
// Build a clip from a uniform sprite-sheet grid
AnimationClip clip = AnimationClip::FromGrid(
    "Run",
    {64, 64},    // frame size in pixels
    /*startFrame=*/0, /*frameCount=*/8,
    /*framesPerRow=*/8, /*frameDuration=*/0.08f, /*bLoop=*/true
);

AnimatorComponent anim;
anim.AddClip(clip);
anim.Play("Run");

world->AddComponent<AnimatorComponent>(entity, anim);
```

Control: `Play(name)`, `Stop()`, `Pause()`, `Resume()`.
`AnimationSystem` runs at `Simulation` priority 5 and writes `SpriteComponent::uvRect`.

---

### `AudioSourceComponent`

```cpp
AudioSourceComponent audio;
audio.FilePath  = "Asset/sfx/jump.wav";
audio.bLooping  = false;
audio.bAutoPlay = true;
world->AddComponent<AudioSourceComponent>(entity, audio);

// Fire-and-forget from World:
world->PlaySound("Asset/sfx/coin.wav");
world->SetGroupVolume(ESoundGroup::SFX, 0.8f);
```

---

### Tags

```cpp
world->AddTag(entity, "Player");
bool isPlayer = world->IsTag(entity, "Player");
auto players  = world->FindEntitiesByTag("Player");
```

---

### `LifeTimeComponent`

```cpp
world->AddComponent<LifeTimeComponent>(entity, /*seconds=*/3.0f);
// Entity is destroyed automatically when lifetime expires.
```

---

## Built-in Systems

| System | Phase | Priority | Purpose |
|---|---|---|---|
| `AnimationSystem` | Simulation | 5 | Drives `AnimatorComponent` → uvRect |
| `PhysicsSyncSystem` | Simulation | — | ECS ↔ PhysicsService sync (creates/destroys bodies, syncs transforms) |
| `CollisionEventDispatchSystem` | Simulation | — | Dispatches collision and trigger enter/stay/exit callbacks |
| `SceneGraphSystem` | PreRender | -5 | Propagates TRS through hierarchy |
| `CameraSystem` | PreRender | 0 | Updates view/projection |
| `RenderSyncSystem` | PreRender | 10 | Submits quads to RenderService |
| `AudioSyncSystem` | Simulation | — | ECS ↔ AudioService sync |
| `LifeTimeSystem` | FrameEnd | — | Destroys expired entities |

> **Physics simulation** (`PhysicsService::Step()`) is driven directly by `World` each fixed update — it is not an ECS system. The ECS systems above only bridge between ECS components and the service.

---

## Key APIs

### Input

```cpp
// Polling — call from OnUpdate()
if (Umbra::Input::GetKey(KeyBoard::W))     { /* held */ }
if (Umbra::Input::GetKeyDown(KeyBoard::Space)) { /* just pressed */ }
if (Umbra::Input::GetKeyUp(KeyBoard::Escape))  { /* just released */ }

if (Umbra::Input::GetMouseButtonDown(Mouse::Left)) {
    Math::Vector2i pos = Umbra::Input::GetMousePosition();
}
```

### Asset Manager

```cpp
// Synchronous load (cached)
SharedPtr<Texture> tex = AssetManager::Get().GetTexture("Asset/player.png");
SharedPtr<Font>    fnt = AssetManager::Get().GetFont("Asset/font.ttf");
SharedPtr<Shader>  sh  = AssetManager::Get().GetShader("Asset/frag.glsl");

// Async load
AssetManager::Get().GetTextureAsync("Asset/bg.png", [](SharedPtr<Texture> t) {
    // use t
});
```

### Scene Navigation

```cpp
// Go to a named scene
SharedPtr<Scene> next = std::make_shared<GameScene>();
GetSceneManager().AddScene("Game", next);
GetSceneManager().GoToScene(next);

// Go with data
SceneContext ctx;
ctx.Set("Score", 1500);
ctx.Set("PlayerName", String("Alice"));
GetSceneManager().GoToScene("GameOver", ctx);

// Receive data in OnBeginPlay()
const SceneContext& ctx = GetSceneContext();
int score = ctx.GetOr<int>("Score", 0);
```

### Camera

```cpp
// Create a default orthographic camera (orthographicSize = half-height in world units)
EntityID cam = CreateDefaultCamera(/*orthographicSize=*/75.f);
SetMainCamera(cam);

// Screen → world
Math::Vector2f worldPos = world->GetScreenToWorldPosition(mouseScreenPos);
```

### Debug Drawing

```cpp
ServiceLocator::GetRenderService()->DebugDrawCircle(pos, radius, bFilled, Color::Green);
ServiceLocator::GetRenderService()->DebugDrawBox(bounds, bFilled, Color::Red);
```

### Prefabs

```cpp
// Register (usually done in Initialize())
PrefabManager::Get().Register("Bullet", [](World* w) -> EntityID {
    EntityID e = w->CreateEntity();
    w->AddComponent<TransformComponent>(e, {0,0}, {8,8});
    // ...
    return e;
});

// Instantiate
EntityID bullet = world->Instantiate("Bullet");
EntityID custom = world->Instantiate("Bullet", [](EntityID e) {
    // override position, color, etc.
});
```

### LDtk Maps

```cpp
// Load an LDtk level as a scene
GetSceneManager().LoadLDTKScene("Asset/map.ldtk", "Level_0");
```

---

## Creating a New Project

### 1. Directory layout

```
Project/MyGame/
├── CMakeLists.txt
├── Asset/           (optional — copied automatically post-build)
└── src/
    ├── MyGameInstance.h
    ├── MyGameInstance.cpp
    ├── MyScene.h
    └── MyScene.cpp
```

### 2. `CMakeLists.txt`

```cmake
file(GLOB_RECURSE SOURCES_MYGAME "src/*.cpp")
add_executable(MyGame ${SOURCES_MYGAME})
target_link_libraries(MyGame Engine2D)
setup_target_dll_copy(MyGame)
setup_target_asset_copy(MyGame)   # only if you have an Asset/ folder
```

### 3. Register the project — root `CMakeLists.txt`

Add this line alongside the other sandboxes:
```cmake
add_subdirectory(Project/MyGame)
```

### 4. Game Instance

```cpp
// MyGameInstance.h
#pragma once
#include "Game/IGameInstance.h"

class MyGameInstance : public Umbra::IGameInstance {
public:
    void Initialize() override;
    void ShutDown()   override;
};
```

```cpp
// MyGameInstance.cpp
#include "MyGameInstance.h"
#include "Game/SceneManager.h"
#include "MyScene.h"

using namespace Umbra;

void MyGameInstance::Initialize() {
    SharedPtr<Scene> scene = std::make_shared<MyScene>();
    GetSceneManager().AddScene("Main", scene);
    GetSceneManager().GoToScene(scene);
}

void MyGameInstance::ShutDown() {}

// Required entry-point function — do not rename
SharedPtr<IGameInstance> CreateApplication() {
    return std::make_shared<MyGameInstance>();
}
```

### 5. Scene

```cpp
// MyScene.h
#pragma once
#include "Game/Scene.h"

class MyScene : public Umbra::Scene {
public:
    void Initialize()   override;
    void OnBeginPlay()  override;
    void OnEndPlay()    override;
    void OnUpdate()     override;
    void OnFixedUpdate() override;
    Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
};
```

```cpp
// MyScene.cpp
#include "MyScene.h"
#include "Asset/AssetManager.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/SpriteQuad.h"
#include "Input/Input.h"

using namespace Umbra;

void MyScene::Initialize() {
    World* world = GetWorld();

    // Camera
    EntityID cam = CreateDefaultCamera(75.f);
    SetMainCamera(cam);

    // Textured sprite
    SharedPtr<Texture> tex = AssetManager::Get().GetTexture("Asset/player.png");
    EntityID player = world->CreateEntity();
    world->AddComponent<TransformComponent>(player, Math::Vector2f{0, 0}, Math::Vector2f{32, 32});
    world->AddComponent<SpriteComponent>(player, tex);
    world->AddTag(player, "Player");
}

void MyScene::OnBeginPlay() {}
void MyScene::OnEndPlay()   {}

void MyScene::OnUpdate() {
    if (Input::GetKey(KeyBoard::W)) {
        // move up
    }
}

void MyScene::OnFixedUpdate() {
    // physics / fixed-rate logic
}

SharedPtr<Scene> MyScene::InstantiateCopy() {
    return std::make_shared<MyScene>();
}
```

---

## Building

### Prerequisites

- CMake 3.5+
- C++17 compiler (MSVC recommended on Windows)
- Git (for submodules)

### Setup

```bash
# 1. Pull all dependencies
git submodule update --init --recursive

# 2. Configure
cmake -S . -B build

# 3. Build
cmake --build build --config Debug
# or
cmake --build build --config Release
```

> **Important:** After adding a new `.cpp` file you must re-run `cmake -S . -B build` because the engine uses `GLOB_RECURSE`.

### Output

Executables land in `build/Project/<ProjectName>/Debug/` (or `Release/`).
Required SFML DLLs are copied automatically on Windows via post-build commands.

---

## Project Structure

```
SFML2D/
├── Engine/
│   ├── Include/
│   │   ├── Asset/          TextureResource, FontResource, ShaderResource, AssetManager
│   │   ├── Core/           App, AppWindow, Clock, Event, Random, Singleton
│   │   ├── Diag/           Logger, Assert, MemoryTracker
│   │   ├── ECS/
│   │   │   ├── Components/ Transform, SpriteQuad, RigidbodyHandle, Collider,
│   │   │   │               CollisionCallback, AnimatorComponent, AudioSource,
│   │   │   │               LifeTime, Tag, CameraComponent, MaterialComponent
│   │   │   └── Systems/    SceneGraphSystem, RenderSyncSystem, AnimationSystem,
│   │   │                   PhysicsSyncSystem, CameraSystem, AudioSyncSystem,
│   │   │                   CollisionEventDispatchSystem, LifeTimeSystem
│   │   ├── FSM/            FiniteStateMachine
│   │   ├── Game/           IGameInstance, Scene, World, SceneManager,
│   │   │                   SceneContext, EntityTemplate, PrefabManager
│   │   ├── Graphics/       ITexture, IFont, IShader, Color, RenderTypes
│   │   ├── Input/          Input (keyboard + mouse)
│   │   ├── LDtk/           LDtkScene, LDtkEntityComponent
│   │   ├── Math/           Vector2f/i, Matrix3x3, Bounds, Box, Polygon,
│   │   │                   Ray, GeometryUtils, MathUtils
│   │   ├── Platform/       VirtualFileManager, NativeFileSystem, PakFileSystem
│   │   ├── Service/
│   │   │   ├── Physics/    PhysicsService, PhysicsBody, PhysicsHandle, Shape,
│   │   │   │               Collision, CollisionQuery, Constraint, DynamicAABBTree,
│   │   │   │               CollisionResolver, PhysicsServiceConfig
│   │   │   ├── Audio/      AudioService, AudioHandle
│   │   │   └── Render/     RenderService, DebugDrawer
│   │   └── Thread/         Thread, Atomic, JobSystem, JobHandle
│   └── src/                Implementation (.cpp files)
│
├── Project/
│   ├── SimpleSandbox/      Basic ECS + sprite demo
│   ├── PhysicsTestBed/     Geometry & raycast tests
│   ├── PhysicsEngineDemo/  Full physics demo
│   ├── AudioTestBed/       Audio playback demo
│   ├── MultiThreading/     Thread / mutex / spinlock demo
│   └── LDtkViewer/         LDtk tile-map viewer
│
└── external/
    ├── SFML/
    ├── imgui / imgui-sfml
    ├── glm
    ├── miniaudio
    └── LDtkLoader
```

---

## Dependencies

| Library | Purpose |
|---|---|
| [SFML](https://www.sfml-dev.org/) | Window, graphics, input, audio back-end |
| [Dear ImGui](https://github.com/ocornut/imgui) + [imgui-sfml](https://github.com/SFML/imgui-sfml) | Immediate-mode debug UI |
| [GLM](https://github.com/g-truc/glm) | Math (supplementary) |
| [miniaudio](https://miniaud.io/) | Cross-platform audio decoding & playback |
| [LDtkLoader](https://github.com/Madour/LDtkLoader) | LDtk level editor file parser |

All managed as git submodules under `external/`.
