# UMBRA Engine Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              GAME LAYER                                     │
│   User Games, Sandboxes, Applications                                       │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                          GAME FRAMEWORK                                     │
│   Scene, World, SceneManager, IGameInstance                                 │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           ECS LAYER                                         │
│   Entity, Component, System, View, ECSRegister                              │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         SERVICES LAYER                                      │
│   Asset, Physics, Input, UI, Audio, FSM                                     │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           CORE LAYER                                        │
│   Logging, Assertion, Math, Random, Color, Singleton                        │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                   PLATFORM ABSTRACTION LAYER (PAL)                          │
│   Window, Clock, Events, Filesystem, Threading, Network                     │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                        EXTERNAL LIBRARIES                                   │
│   SFML, ImGui, GLM, miniaudio, PCG                                          │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Layer Details

### GAME LAYER
User-created games and applications that use the engine.

| Module           | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| SimpleSandbox    | Implemented | Basic testing sandbox                  |
| PhysicsTestBed   | Implemented | Physics demonstration application      |
| AudioTestBed     | Implemented | Audio demo (SFX, music, volume mixing) |
| MultiThreading   | Implemented | Thread, Mutex, SpinLock, CV demo Job pool: Submit, SubmitAfter, ParallelFor, throughput graph     |
| LDtkViewer       | Implemented | LDtk level viewer application          |

---

### GAME FRAMEWORK
High-level game abstractions for scene and world management.

**📖 See detailed design: [docs/GameFrameworkDesign.md](docs/GameFrameworkDesign.md)**

| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| IGameInstance    | Implemented | Base class for game applications       |
| Scene            | Implemented | Runtime scene with lifecycle methods   |
| SceneManager     | Implemented | Scene lifecycle and transitions        |
| World            | Implemented | ECS world container per scene          |
| App              | Implemented | Main application loop                  |
| SceneContext     | Implemented | Key-value bag (`std::any`) passed via GoToScene, read via GetSceneContext() |
| SceneLoadOptions | Planned     | Scene loading configuration            |
| ESceneState      | Planned     | Scene state enumeration                |
| SceneAsset       | Stub        | Scene template/prefab                  |
| EntityTemplate   | Implemented | Factory alias: `std::function<EntityID(World&)>` |
| PrefabManager    | Implemented | Global singleton: Register, Has, Instantiate (with optional override), Clear |
| LDtkScene        | Implemented | Async LDtk level loader — parses .ldtk, loads tilesets, spawns ECS entities; register via `SceneManager::LoadLDTKScene()` |
| LayerManager     | Planned     | Scene layer system                     |
| SaveSystem       | Planned     | Game state persistence                 |

---

### ECS LAYER
Entity Component System architecture for game object management.

#### Core ECS
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| EntityID         | Implemented | Lightweight entity identifier          |
| EntityManager    | Implemented | Entity lifecycle management            |
| ComponentArray   | Implemented | Sparse set component storage           |
| ComponentManager | Implemented | Component array registry               |
| System           | Implemented | Base class for all systems             |
| ECSRegister      | Implemented | Central ECS management                 |
| ECView           | Implemented | Entity queries by component signature  |
| SystemManager    | Implemented | Typed system lookup, enable/disable, entity notifications |

#### Built-in Components
| Component             | Status      | Description                       |
|-----------------------|-------------|-----------------------------------|
| TransformComponent    | Implemented | Position, rotation, scale, pivot  |
| SpriteComponent       | Implemented | Sprite rendering data             |
| AnimatorComponent     | Implemented | Sprite sheet animation state      |
| PhysicsBodyComponent  | Implemented | Mass, forces, torque, inertia     |
| RigidBodyComponent    | Implemented | Simple velocity-based physics     |
| CameraComponent       | Implemented | Orthographic camera               |
| BoxColliderComponent  | Implemented | Box collision shape               |
| CircleColliderComponent| Implemented | Circle collision shape           |
| CapsuleCollider       | Stub        | Capsule collision shape           |
| BoundingVolumeAABB    | Implemented | AABB for broadphase               |
| LifeTimeComponent     | Implemented | Entity lifetime management        |
| TagComponent          | Implemented | String tag identification         |
| CollisionCallbackComponent| Implemented | Enter/Stay/Exit collision callbacks |
| RigidbodyHandleComponent| Implemented | Bridge to PhysicsService via handle |
| AudioSourceComponent  | Implemented | Bridge to AudioService via handle |
| LDtkEntityComponent   | Implemented | LDtk entity-layer entity: typeName + fields map |
| MaterialComponent     | Implemented | Optional shader override for a sprite entity; read by RenderSyncSystem |
| CollisionEventComponent| Deprecated | Event-based collision (removed)  |

#### Built-in Systems
| System               | Status      | Phase       | Pri | Description                |
|----------------------|-------------|-------------|-----|----------------------------|
| CameraSystem         | Implemented | PreRender   | 0   | Camera view management, notifies RenderService |
| RenderSyncSystem     | Implemented | PreRender   | 10  | Submits RenderQuads to RenderService; reads MaterialComponent shader if present |
| PhysicsSystem        | Implemented | Simulation  | 0   | Legacy physics simulation  |
| PhysicsSyncSystem    | Implemented | Simulation  | 10  | ECS ↔ PhysicsService sync  |
| CollisionEventDispatchSystem | Implemented | Simulation | 20 | Dispatches enter/stay/exit callbacks |
| LifeTimeSystem       | Implemented | FrameEnd    | -   | Entity destruction by time |
| CollisionDetectionSystem | Deprecated | - | -   | Old collision (removed)   |
| CollisionEventResolverSystem | Deprecated | - | - | Event collision (removed)|
| AnimationSystem      | Implemented | Simulation  | 5   | Sprite sheet animation     |
| ParticleSystem       | DO NOT IMPLEMENT     | Simulation  | -   | Particle effects           |
| AudioSyncSystem      | Implemented | Simulation  | 30  | ECS <-> AudioService sync  |
| SceneGraphSystem     | Implemented | PreRender   | -5  | Local→World matrix propagation (TRS DFS) |
| ScriptSystem         | Planned     | Simulation  | -   | Scripting support          |

---

### SERVICES LAYER
Engine services and subsystems.

#### Asset Management
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| AssetManager     | Implemented | Singleton resource cache; GetTexture/Font/Shader + async variants |
| IResource        | Implemented | Resource interface                     |
| TextureResource  | Implemented | Texture loader via ITexture            |
| Texture          | Implemented | Texture wrapper                        |
| AssetRegister    | Implemented | Resource registry                      |
| IResourceHandle  | Implemented | Resource handle interface              |
| AudioResource    | Implemented | Audio file resource (miniaudio decoder) |
| Audio            | Implemented | Audio handle wrapper                   |
| FontResource     | Implemented | Font file resource (via IFont)         |
| Font             | Implemented | Font handle wrapper                    |
| ShaderResource   | Implemented | GLSL shader loader (frag-only or vert+frag via `|`-separated path key) |
| Shader           | Implemented | Shader handle wrapper; uniform setters delegate to IShader |

#### Render Service
| Class                     | Status      | Description                     |
|---------------------------|-------------|---------------------------------|
| RenderService             | Implemented | Quad queue, debug draw, frustum cull |
| RenderTypes (RenderQuad)  | Implemented | Render quad: Position, Size, Pivot, Angle, ZOrder, Tint, Texture, UVRect, Shader |
| RenderStats               | Implemented | Per-frame render statistics     |
| DebugDrawer               | Implemented | Debug primitive accumulator     |

**Pipeline:** `BeginFrame()` (clear) → `RenderSyncSystem` submits quads → `Flush()` (sort, cull, batch, debug draw). Accessed globally via `ServiceLocator::GetRenderService()`.

#### Physics Service — Parallel Step Pipeline

`PhysicsService::Step()` is wired to the `JobSystem` via `SetJobSystem()` (called in `ServiceLocator::Initialize()` after both services are created). When body or pair counts exceed the dispatch thresholds, the following phases run in parallel across all worker threads:

| Phase                        | Parallelized | Threshold       | Notes |
|------------------------------|:------------:|-----------------|-------|
| `IntegrateForces`            | Yes          | 64 bodies       | Per-body, fully independent |
| `SaveCCDState`               | Yes          | 64 bodies       | Per-body, fully independent |
| `IntegrateVelocities`        | Yes          | 64 bodies       | Per-body, fully independent |
| `ApplyDamping`               | Yes          | 64 bodies       | Per-body, fully independent |
| `ClearForceAccumulators`     | Yes          | 64 bodies       | Per-body, fully independent |
| `NarrowPhaseDetection`       | Yes          | 32 pairs        | Atomic write-index into pre-allocated vector |
| `PrecomputeContactConstraints` | Yes        | 32 collisions   | Each collision writes only to its own contacts |
| `UpdateBroadphaseProxies`    | No           | —               | Mutates DynamicAABBTree structure |
| `BroadphaseDetection`        | No           | —               | Single-threaded tree query + pair list build |
| `ApplySpringForces`          | No           | —               | Springs share body force accumulators |
| `ResolveContacts` (velocity) | No           | —               | Sequential impulse: contacts share body velocities |
| `PositionContraction`        | No           | —               | Contacts share body positions |
| `UpdateSleepingBodies`       | No           | —               | Cross-body wake logic based on collision pairs |
| `CategorizeCollisionEvents`  | No           | —               | Reads/writes shared event sets |

Below the thresholds, each phase falls back to a sequential loop with identical behavior.

#### Physics Service
| Class                     | Status      | Description                     |
|---------------------------|-------------|---------------------------------|
| PhysicsService            | Implemented | Handle-based physics simulation |
| PhysicsServiceConfig      | Implemented | Physics configuration           |
| PhysicsBodyData           | Implemented | Internal body storage           |
| BodyHandle                | Implemented | Generational index handle       |
| BodyDef                   | Implemented | Body creation definition        |
| CollisionDef              | Implemented | Per-frame collision data        |
| CollisionEvent            | Implemented | Enter/stay/exit event data      |
| CollisionQuery            | Implemented | SAT narrow-phase detection      |
| DynamicAABBTree           | Implemented | Broadphase acceleration tree    |
| RaycastHit                | Implemented | Ray query result                |
| CollisionDetector         | Implemented | Legacy broad/narrow phase       |
| ContactResolver           | Implemented | Legacy impulse resolution       |
| IBroadphaseResolver       | Implemented | Legacy broadphase interface     |
| BruteForceBroadphaseResolver | Implemented | Legacy O(n^2) broadphase    |
| IForceGenerator           | Implemented | Force generator interface       |
| ITorqueGenerator          | Implemented | Torque generator interface      |
| SpringForceGenerator      | Implemented | Spring physics                  |
| SpatialHashBroadphase     |  DO NOT IMPLEMENT       | Spatial hash broadphase         |
| QuadTreeBroadphase        |  DO NOT IMPLEMENT       | Quadtree broadphase             |
| GravityForceGenerator     |  DO NOT IMPLEMENT       | Gravity force generator         |
| DragForceGenerator        |  DO NOT IMPLEMENT       | Drag force generator            |

#### Input Service
| Class                   | Status      | Description                       |
|-------------------------|-------------|-----------------------------------|
| Input                   | Implemented | Singleton input manager           |
| KeyPressedEvent         | Implemented | Key press event                   |
| KeyReleasedEvent        | Implemented | Key release event                 |
| MouseButtonPressedEvent | Implemented | Mouse button press event          |
| MouseButtonReleasedEvent| Implemented | Mouse button release event        |
| MouseMovedEvent         | Implemented | Mouse movement event              |
| GamepadInput            | Planned     | Gamepad/controller support        |
| InputMapping            | DO NOT IMPLEMENT     | Action-based input mapping        |

#### UI Service
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| ImGuiBackend     | Implemented | ImGui backend interface                |
| SfmlImguiImpl    | Implemented | SFML-ImGui implementation              |
| UIManager        | Implemented | UI management                          |
| Canvas           | Planned     | In-game UI canvas                      |
| Widget           | Planned     | Base UI widget class                   |

#### Audio Service
| Class                  | Status      | Description                            |
|------------------------|-------------|----------------------------------------|
| AudioService           | Implemented | Handle-based audio playback via miniaudio |
| AudioServiceConfig     | Implemented | Audio configuration (volumes, capacity)  |
| SoundData              | Implemented | Internal sound storage                   |
| SoundHandle            | Implemented | Generational index handle                |
| ESoundGroup            | Implemented | Master/SFX/Music volume groups           |
| AudioListener          | Planned     | Audio listener component (spatial audio) |

#### State Machine
| Class              | Status      | Description                          |
|--------------------|-------------|--------------------------------------|
| FiniteStateMachine | Implemented | FSM manager                          |
| IFSMState          | Implemented | FSM state interface                  |

---

### CORE LAYER
Fundamental utilities and data structures.

#### Diagnostics
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| Logger           | Implemented | Multi-level logging system             |
| Assert           | Implemented | Assertion macros                       |
| Profiler         | Planned     | Performance profiling                  |
| MemoryTracker    | Implemented | Per-category heap tracking via global new/delete overrides; `EMemoryCategory` enum (General/ECS/Physics/Asset/Audio/UI); `UMBRA_ALLOC_SCOPE(cat)` RAII macro; ImGui panel |

#### Math
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| TVector          | Implemented | Generic 2D vector template             |
| Vector2f/2i/2lf  | Implemented | Vector type aliases                    |
| Bounds2D         | Implemented | Axis-aligned bounding box              |
| Box              | Implemented | Oriented bounding box                  |
| Polygon          | Implemented | 2D polygon                             |
| Triangle         | Implemented | Triangle primitive                     |
| Ray              | Implemented | Ray primitive                          |
| MathUtils        | Implemented | Math utility functions                 |
| GeometryUtils    | Implemented | Geometry utilities                     |
| CollisionSystem  | Implemented | Collision math algorithms              |
| Matrix3x3        | Implemented | Row-major affine 3×3, TRS/Decompose, header-only |

#### Graphics
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| Color            | Implemented | RGBA color with GLM backend            |
| IRenderDevice    | Implemented | Abstract rendering interface           |
| ITexture         | Implemented | Abstract texture interface             |
| IFont            | Implemented | Abstract font interface                |
| IShader          | Implemented | Abstract shader interface (frag/vert+frag load, uniform setters) |
| RenderTypes      | Implemented | FloatRect, Vertex, EWindowEvent        |
| SfmlRenderDevice | Implemented | SFML backend for IRenderDevice         |
| SfmlTexture      | Implemented | SFML backend for ITexture              |
| SfmlShader       | Implemented | SFML backend for IShader (wraps sf::Shader) |

#### Utilities
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| Singleton        | Implemented | Singleton pattern template             |
| Random           | Implemented | PCG-based random number generator      |
| SparseArray      | Stub        | Sparse set template                    |
| ObjectPool       | Planned     | Object pooling                         |
| StringUtils      | Planned     | String manipulation utilities          |
| PathUtils        | Implemented | Header-only (`Util/PathUtils.h`): Normalize, GetDirectory, GetFileName, GetStem, GetExtension, Join, ChangeExtension, GetAbsolute, GetRelative, IsAbsolute, Exists, IsFile, IsDirectory |

---

### PLATFORM ABSTRACTION LAYER (PAL)
Platform-specific abstractions.

#### Window & Display
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| AppWindow        | Implemented | Window management via IRenderDevice    |
| AppClosedEvent   | Implemented | Window close event                     |
| NativeWindowEvent| Implemented | Backend-agnostic native event wrapper  |

#### Time
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| Clock            | Implemented | std::chrono-based clock                |
| EngineTime       | Implemented | Global time utilities                  |

#### Events
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| EventBus         | Implemented | Event dispatch system                  |
| Delegate         | Implemented | Generic callback delegate              |

#### Filesystem
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| FileSystem       | Implemented     | File I/O abstraction                   |
| FileReader       | Implemented     | File reading utilities                 |
| FileWriter       | Implemented     | File writing utilities                 |
| PathUtils        | Implemented     | See Utilities section                  |

#### Threading
| Class              | Status      | Description                                          |
|--------------------|-------------|------------------------------------------------------|
| Thread             | Implemented | RAII thread wrapper with platform name support       |
| Mutex              | Implemented | OS-level blocking lock (wraps std::mutex)            |
| SpinLock           | Implemented | User-space busy-wait lock (atomic_flag)              |
| LockGuard          | Implemented | RAII scoped lock (Mutex or SpinLock)                 |
| UniqueLock         | Implemented | RAII scoped lock with manual unlock (for CV)         |
| ConditionVariable  | Implemented | Sleep/wake synchronisation with predicate support    |
| Atomic\<T\>        | Implemented | Typed atomic with EMemoryOrder enum                  |
| JobHandle          | Implemented | Lightweight completion token (poll or wait)          |
| JobCompletion      | Implemented | Shared ref-counted counter + callback list           |
| JobSystem          | Implemented | Fixed thread pool: Submit, SubmitAfter, ParallelFor  |

**JobSystem API summary:**

```cpp
// M1/M2 — fire and poll
JobHandle h = js->Submit([](){ DoWork(); });
h.Wait();

// M3 — chain A → B → C (B runs after A, C after B)
JobHandle a = js->Submit(TaskA);
JobHandle b = js->SubmitAfter(a, TaskB);
JobHandle c = js->SubmitAfter(b, TaskC);
c.Wait();

// M4 — parallel array fill (fork-join)
js->ParallelFor(1000, [&](uint32 i){ results[i] = i * i; }).Wait();
```

**Key design decisions:**
- `JobCompletion::mPendingCount` starts at 1 (Submit) or N (ParallelFor); Acquire/Release ordering ensures job writes are visible after `IsComplete()` returns true.
- Dependency wakeup is callback-based: `SubmitAfter` appends an `EnqueueRaw` lambda to the parent's callback list under the callback mutex, preventing a race with `Decrement()`.
- `ParallelFor` pushes all N items under a single lock then calls `NotifyAll()` to wake all workers simultaneously.
- Registered via `ServiceLocator::GetJobSystem()`; initialized in `ServiceLocator::Initialize()`, shut down last in `ServiceLocator::Shutdown()`.

#### Network
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| Socket           | Planned     | Socket abstraction                     |
| TCPClient        | Planned     | TCP client                             |
| TCPServer        | Planned     | TCP server                             |
| UDPSocket        | Planned     | UDP socket                             |

---

### EXTERNAL LIBRARIES

| Library      | Version | Purpose                                    |
|--------------|---------|-------------------------------------------|
| SFML         | 2.x     | Graphics, Window, Input, Audio            |
| ImGui        | -       | Immediate mode GUI                        |
| ImGui-SFML   | -       | SFML backend for ImGui                    |
| GLM          | -       | Math library (vectors, matrices)          |
| miniaudio    | 0.11    | Audio playback (SFX, music, spatial)      |
| PCG          | -       | Random number generation                  |
| LDtkLoader   | -       | LDtk level JSON parser (Madour/LDtkLoader)|

---

## System Execution Phases

```
┌──────────────┐
│  FrameStart  │  Input collection, event processing
└──────┬───────┘
       ▼
┌──────────────┐
│  Simulation  │  Physics, AI, game logic
└──────┬───────┘
       ▼
┌──────────────┐
│  PreRender   │  Camera updates, quad submission to RenderService
└──────┬───────┘
       ▼
┌──────────────┐
│   FrameEnd   │  Cleanup, entity destruction
└──────────────┘

Note: Actual rendering (sort, cull, batch, draw) happens in
RenderService::Flush() called from App::OnUpdate() after all
ECS phases complete but before ImGui and display refresh.
```

---

## Directory Structure

```
Engine/
├── Include/
│   ├── Core/              App, AppWindow, Clock, Event, Random, Singleton
│   ├── Diag/              Logger, Assert
│   ├── ECS/               Component, System, View, ECSRegister
│   │   ├── Components/    Transform, Sprite, Physics, Camera, Collider
│   │   └── Systems/       RenderSyncSystem, Physics, Camera, LifeTime
│   ├── Game/              IGameInstance, Scene, SceneManager, World
│   ├── Math/              Vector, Bounds, Box, Polygon, Collision
│   ├── Physics/           Collision, CollisionDetector, ContactResolver
│   ├── Service/
│   │   ├── Audio/         AudioService, AudioHandle, AudioServiceConfig
│   │   ├── Render/        RenderService, DebugDrawer, RenderTypes
│   │   └── Physics/       PhysicsService, PhysicsBody, PhysicsServiceConfig
│   ├── Asset/             AssetManager, TextureResource, Texture
│   ├── LDtk/              LDtkScene, LDtkEntityComponent
│   ├── Input/             Input, Keyboard/Mouse events
│   ├── Graphics/          Color, IRenderDevice, ITexture, IFont, IShader, RenderTypes
│   │   └── Backends/      SfmlRenderDevice, SfmlTexture, SfmlShader
│   ├── UI/                ImGuiBackend, UIManager
│   ├── FSM/               FiniteStateMachine, IFSMState
│   ├── Types/             SparseArray
│   ├── Util/              PathUtils (header-only)
│   └── Umbra.h            Main include header
├── src/                   Implementation files (mirrors Include structure)
└── EnginePCH.h            Precompiled header with type aliases
```

---

## Legend

| Status      | Meaning                                      |
|-------------|----------------------------------------------|
| Implemented | Fully functional                             |
| Stub        | Header exists, minimal/no implementation     |
| Planned     | Not yet implemented, future feature          |
| Deprecated  | Removed or disabled, may be deleted          |
