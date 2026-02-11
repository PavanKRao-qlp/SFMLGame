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
| SceneContext     | Planned     | Data passed between scenes             |
| SceneLoadOptions | Planned     | Scene loading configuration            |
| ESceneState      | Planned     | Scene state enumeration                |
| SceneAsset       | Stub        | Scene template/prefab                  |
| EntityTemplate   | Planned     | Entity prefab system                   |
| PrefabManager    | Planned     | Prefab registration and instantiation  |
| LayerManager     | Planned     | Scene layer system                     |
| SaveSystem       | Planned     | Game state persistence                 |

**Known Issues (Priority: High):**
- ~~World::RemoveComponent bug~~ (Fixed)
- LoadGameConfig returns reference to temporary
- OnEndPlay not called during scene transitions
- UI rendering coupled to Scene::Render
- Hardcoded camera creation in all scenes
- Naming typos: InsatiateCopy, OnFixedUpdated, GetSceneManger

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
| SystemManager    | Stub        | System management utilities            |

#### Built-in Components
| Component             | Status      | Description                       |
|-----------------------|-------------|-----------------------------------|
| TransformComponent    | Implemented | Position, rotation, scale, pivot  |
| SpriteComponent       | Implemented | Sprite rendering data             |
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
| CollisionEventComponent| Deprecated | Event-based collision (removed)  |

#### Built-in Systems
| System               | Status      | Phase       | Pri | Description                |
|----------------------|-------------|-------------|-----|----------------------------|
| RenderSystem         | Implemented | Render      | 0   | Sprite rendering           |
| PhysicsSystem        | Implemented | Simulation  | 0   | Legacy physics simulation  |
| PhysicsSyncSystem    | Implemented | Simulation  | 10  | ECS ↔ PhysicsService sync  |
| CollisionEventDispatchSystem | Implemented | Simulation | 20 | Dispatches enter/stay/exit callbacks |
| CameraSystem         | Implemented | PreRender   | 0   | Camera view management     |
| LifeTimeSystem       | Implemented | FrameEnd    | -   | Entity destruction by time |
| CollisionDetectionSystem | Deprecated | - | -   | Old collision (removed)   |
| CollisionEventResolverSystem | Deprecated | - | - | Event collision (removed)|
| AnimationSystem      | Planned     | Simulation  | -   | Sprite animation           |
| ParticleSystem       | Planned     | Simulation  | -   | Particle effects           |
| AudioSystem          | Planned     | Simulation  | -   | Spatial audio              |
| ScriptSystem         | Planned     | Simulation  | -   | Scripting support          |

---

### SERVICES LAYER
Engine services and subsystems.

#### Asset Management
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| AssetManager     | Implemented | Singleton resource cache               |
| IResource        | Implemented | Resource interface                     |
| TextureResource  | Implemented | Texture loader via ITexture            |
| Texture          | Implemented | Texture wrapper                        |
| AssetRegister    | Implemented | Resource registry                      |
| IResourceHandle  | Stub        | Resource handle interface              |
| AudioResource    | Planned     | Audio file resource                    |
| FontResource     | Planned     | Font file resource                     |
| ShaderResource   | Planned     | Shader program resource                |

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
| SpatialHashBroadphase     | Planned     | Spatial hash broadphase         |
| QuadTreeBroadphase        | Planned     | Quadtree broadphase             |
| GravityForceGenerator     | Planned     | Gravity force generator         |
| DragForceGenerator        | Planned     | Drag force generator            |

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
| InputMapping            | Planned     | Action-based input mapping        |

#### UI Service
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| ImGuiBackend     | Implemented | ImGui backend interface                |
| SfmlImguiImpl    | Implemented | SFML-ImGui implementation              |
| UIManager        | Implemented | UI management                          |
| Canvas           | Planned     | In-game UI canvas                      |
| Widget           | Planned     | Base UI widget class                   |

#### Audio Service
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| AudioManager     | Planned     | Audio playback management              |
| AudioSource      | Planned     | Positional audio source                |
| AudioListener    | Planned     | Audio listener component               |
| SoundEffect      | Planned     | One-shot sound effects                 |
| MusicTrack       | Planned     | Streaming music playback               |

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
| MemoryTracker    | Planned     | Memory allocation tracking             |

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
| Matrix3x3        | Planned     | 3x3 transformation matrix              |

#### Graphics
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| Color            | Implemented | RGBA color with GLM backend            |
| IRenderDevice    | Implemented | Abstract rendering interface           |
| ITexture         | Implemented | Abstract texture interface             |
| RenderTypes      | Implemented | FloatRect, Vertex, EWindowEvent        |
| SfmlRenderDevice | Implemented | SFML backend for IRenderDevice         |
| SfmlTexture      | Implemented | SFML backend for ITexture              |

#### Utilities
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| Singleton        | Implemented | Singleton pattern template             |
| Random           | Implemented | PCG-based random number generator      |
| SparseArray      | Stub        | Sparse set template                    |
| ObjectPool       | Planned     | Object pooling                         |
| StringUtils      | Planned     | String manipulation utilities          |

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
| PathUtils        | Planned     | Path manipulation                      |

#### Threading
| Class            | Status      | Description                            |
|------------------|-------------|----------------------------------------|
| ThreadPool       | Planned     | Worker thread pool                     |
| JobSystem        | Planned     | Job scheduling                         |
| Mutex            | Planned     | Mutex wrapper                          |
| Atomic           | Planned     | Atomic operations                      |

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
| miniaudio    | -       | Audio library (planned integration)       |
| PCG          | -       | Random number generation                  |

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
│  PreRender   │  Visibility culling, camera updates
└──────┬───────┘
       ▼
┌──────────────┐
│    Render    │  Drawing, debug visualization
└──────┬───────┘
       ▼
┌──────────────┐
│   FrameEnd   │  Cleanup, entity destruction
└──────────────┘
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
│   │   └── Systems/       Render, Physics, Camera, LifeTime
│   ├── Game/              IGameInstance, Scene, SceneManager, World
│   ├── Math/              Vector, Bounds, Box, Polygon, Collision
│   ├── Physics/           Collision, CollisionDetector, ContactResolver
│   ├── Asset/             AssetManager, TextureResource, Texture
│   ├── Input/             Input, Keyboard/Mouse events
│   ├── Graphics/          Color, IRenderDevice, ITexture, RenderTypes
│   │   └── Backends/      SfmlRenderDevice, SfmlTexture
│   ├── UI/                ImGuiBackend, UIManager
│   ├── FSM/               FiniteStateMachine, IFSMState
│   ├── Types/             SparseArray
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
