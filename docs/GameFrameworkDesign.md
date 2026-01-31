# UMBRA Game Framework Layer - Design Document

## Executive Summary

This document outlines a comprehensive redesign of UMBRA's Game Framework layer to provide a robust, flexible, and game-developer-friendly API for building 2D games. The design addresses current architectural issues while adding modern game engine features.

---

## Table of Contents

1. [Design Philosophy](#design-philosophy)
2. [Architecture Overview](#architecture-overview)
3. [Core Components](#core-components)
4. [Advanced Features](#advanced-features)
5. [API Examples](#api-examples)
6. [Migration Guide](#migration-guide)
7. [Implementation Roadmap](#implementation-roadmap)

---

## Design Philosophy

### Core Principles

1. **Separation of Concerns** - Each layer has a clear responsibility
2. **Ownership Clarity** - Explicit ownership model (unique vs shared)
3. **Extensibility** - Easy to extend without modifying core code
4. **Developer Ergonomics** - Intuitive API that guides developers naturally
5. **Performance First** - Zero-cost abstractions where possible
6. **Data-Driven** - Support for serialization and asset pipelines

### Key Improvements Over Current Design

- Fixed critical bugs (RemoveComponent, LoadGameConfig)
- Clear scene lifecycle with proper cleanup
- Scene state management (active/paused/suspended)
- Scene data passing and context management
- Asset preloading and streaming
- Prefab/entity template system
- Layer-based rendering and update ordering
- Game state persistence
- Modular system architecture

---

## Architecture Overview

### Layer Hierarchy

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                     │
│  ┌───────────────────────────────────────────────────┐  │
│  │              IGameInstance                        │  │
│  │  - Global game state                              │  │
│  │  - Configuration                                  │  │
│  │  - Service registration                           │  │
│  └───────────────────────────────────────────────────┘  │
│                           │                              │
│  ┌───────────────────────────────────────────────────┐  │
│  │              SceneManager                         │  │
│  │  - Scene lifecycle management                     │  │
│  │  - Scene transitions                              │  │
│  │  - Scene stack (overlays)                         │  │
│  │  - Asset preloading                               │  │
│  └───────────────────────────────────────────────────┘  │
│                           │                              │
│  ┌───────────────────────────────────────────────────┐  │
│  │              Scene (Runtime)                      │  │
│  │  - Scene-local state                              │  │
│  │  - Scene layers                                   │  │
│  │  - Entity management delegation                   │  │
│  └───────────────────────────────────────────────────┘  │
│                           │                              │
│  ┌───────────────────────────────────────────────────┐  │
│  │              World (ECS Container)                │  │
│  │  - Entity registry                                │  │
│  │  - Component storage                              │  │
│  │  - System execution                               │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
                           │
┌─────────────────────────────────────────────────────────┐
│                    Engine Services                       │
│  - AssetManager  - Input  - Audio  - Physics  - Render  │
└─────────────────────────────────────────────────────────┘
```

### Ownership Model

```cpp
// Clear ownership hierarchy
IGameInstance (owned by App)
    └─> UniquePtr<SceneManager>
            ├─> UMap<String, UniquePtr<SceneAsset>>  // Templates
            ├─> Vector<UniquePtr<Scene>>             // Active stack
            └─> Scene* mActiveScene                  // Current focus
                    └─> UniquePtr<World>
                            └─> SharedPtr<ECSRegister>
```

**Key Points:**
- Use `UniquePtr` for exclusive ownership
- Use `SharedPtr` only when multiple owners genuinely needed (systems, resources)
- Use raw pointers for non-owning references
- No `std::move()` on SharedPtr (semantic error)

---

## Core Components

### 1. IGameInstance (Application Entry Point)

**Responsibilities:**
- Initialize game-wide configuration
- Register scenes as templates
- Register global services (custom systems, managers)
- Handle application-level events (window focus, minimize, etc.)
- Manage persistent game state across scenes

**Interface:**

```cpp
namespace Umbra {

    struct FGameConfig {
        String windowTitle = "UMBRA Game";
        uint32 windowWidth = 1280;
        uint32 windowHeight = 720;
        bool bFullscreen = false;
        bool bVSync = true;
        float fixedDeltaTime = 0.016f;     // 60 FPS physics
        float maxPhysicsDelta = 0.1f;
        uint32 targetFrameRate = 60;       // 0 = uncapped
    };

    class IGameInstance {
    public:
        virtual ~IGameInstance() = default;

        // Core lifecycle
        virtual FGameConfig LoadGameConfig() = 0;
        virtual void Initialize() = 0;
        virtual void Shutdown() = 0;

        // Optional override: called every frame (before scenes)
        virtual void OnUpdate(float _deltaTime) {}

        // Optional override: application events
        virtual void OnApplicationFocusChanged(bool _hasFocus) {}
        virtual void OnApplicationPaused() {}
        virtual void OnApplicationResumed() {}

        // Scene management
        SceneManager& GetSceneManager();

        // Global service access
        AssetManager& GetAssetManager();
        Input& GetInput();
        AudioManager& GetAudioManager();

        // Application control
        void RequestExit();
        bool IsExitRequested() const;

    protected:
        IGameInstance();
        IGameInstance(const IGameInstance&) = delete;
        IGameInstance& operator=(const IGameInstance&) = delete;

    private:
        friend class App;
        void SetSceneManager(UniquePtr<SceneManager> _sceneManager);

        UniquePtr<SceneManager> mSceneManager;
        bool bExitRequested = false;
    };

} // namespace Umbra

// Required in each game project
extern Umbra::UniquePtr<Umbra::IGameInstance> CreateApplication();
```

**Example Usage:**

```cpp
class MyGameInstance : public IGameInstance {
public:
    FGameConfig LoadGameConfig() override {
        FGameConfig config;
        config.windowTitle = "My Awesome Game";
        config.windowWidth = 1920;
        config.windowHeight = 1080;
        return config;
    }

    void Initialize() override {
        // Register scene templates
        GetSceneManager().RegisterSceneAsset("MainMenu",
            MakeUnique<MainMenuScene>());
        GetSceneManager().RegisterSceneAsset("Level1",
            MakeUnique<Level1Scene>());
        GetSceneManager().RegisterSceneAsset("Level2",
            MakeUnique<Level2Scene>());

        // Start with main menu
        GetSceneManager().LoadScene("MainMenu");
    }

    void Shutdown() override {
        // Save game state, cleanup
        SavePlayerProgress();
    }

    void OnUpdate(float _deltaTime) override {
        // Handle global input (pause menu, screenshot, etc.)
        if (Input::GetKeyDown(KeyBoard::F11)) {
            ToggleFullscreen();
        }
    }

private:
    void SavePlayerProgress() { /* ... */ }
    void ToggleFullscreen() { /* ... */ }
};

// Entry point
UniquePtr<IGameInstance> CreateApplication() {
    return MakeUnique<MyGameInstance>();
}
```

---

### 2. SceneManager (Scene Lifecycle & Transitions)

**Responsibilities:**
- Manage scene templates (SceneAssets)
- Handle scene instantiation and transitions
- Support scene stack for overlays (pause menu over gameplay)
- Preload scenes asynchronously
- Pass data between scenes via SceneContext

**Interface:**

```cpp
namespace Umbra {

    // Data passed between scenes
    struct SceneContext {
        UMap<String, String> stringParams;
        UMap<String, int32> intParams;
        UMap<String, float> floatParams;
        UMap<String, bool> boolParams;

        // Helper methods
        void SetString(const String& _key, const String& _value);
        String GetString(const String& _key, const String& _default = "") const;
        void SetInt(const String& _key, int32 _value);
        int32 GetInt(const String& _key, int32 _default = 0) const;
        // ... similar for float, bool
    };

    enum class ESceneTransition {
        None,           // Instant switch
        FadeOut,        // Fade to black then load
        FadeInOut,      // Fade out old, fade in new
        Slide,          // Slide transition
        Custom          // User-defined transition
    };

    struct SceneLoadOptions {
        ESceneTransition transition = ESceneTransition::None;
        float transitionDuration = 0.5f;
        SceneContext context;
        bool bUnloadCurrent = true;     // False for overlays
        bool bPauseCurrent = true;      // Pause scene below overlay
    };

    class SceneManager {
    public:
        SceneManager();
        ~SceneManager();

        // Scene template registration
        void RegisterSceneAsset(const String& _sceneId,
                                UniquePtr<Scene> _sceneTemplate);
        bool HasSceneAsset(const String& _sceneId) const;
        void UnregisterSceneAsset(const String& _sceneId);

        // Scene loading
        void LoadScene(const String& _sceneId,
                       const SceneLoadOptions& _options = {});
        void LoadSceneAdditive(const String& _sceneId,
                               const SceneLoadOptions& _options = {});
        void UnloadScene(const String& _sceneId);

        // Preloading
        void PreloadScene(const String& _sceneId);
        bool IsScenePreloaded(const String& _sceneId) const;
        float GetPreloadProgress(const String& _sceneId) const;

        // Scene stack management
        Scene* GetActiveScene();
        const Scene* GetActiveScene() const;
        Scene* GetSceneByIndex(size_t _index);
        size_t GetSceneCount() const;

        // Scene state control
        void PauseScene(Scene* _scene);
        void ResumeScene(Scene* _scene);

        // Update
        void Update(float _deltaTime);
        void FixedUpdate(float _fixedDelta);
        void Render();

        // Shutdown
        void Shutdown();

    private:
        friend class IGameInstance;
        void SetGameInstance(IGameInstance* _gameInstance);

        UniquePtr<Scene> InstantiateScene(const String& _sceneId);
        void ExecuteSceneTransition(UniquePtr<Scene> _newScene,
                                    const SceneLoadOptions& _options);

        IGameInstance* mGameInstance = nullptr;

        // Scene templates (master copies)
        UMap<String, UniquePtr<Scene>> mSceneAssets;

        // Active scene stack (instantiated scenes)
        Vector<UniquePtr<Scene>> mSceneStack;
        Scene* mActiveScene = nullptr;

        // Preloading
        UMap<String, UniquePtr<Scene>> mPreloadedScenes;

        // Transition state
        bool bInTransition = false;
        float mTransitionTimer = 0.0f;
    };

} // namespace Umbra
```

**Example Usage:**

```cpp
// Simple scene switch
GetSceneManager().LoadScene("Level1");

// Scene switch with fade transition
SceneLoadOptions options;
options.transition = ESceneTransition::FadeInOut;
options.transitionDuration = 1.0f;
options.context.SetInt("levelNumber", 1);
options.context.SetInt("playerHealth", 100);
GetSceneManager().LoadScene("Level1", options);

// Load pause menu as overlay
SceneLoadOptions pauseOptions;
pauseOptions.bUnloadCurrent = false;  // Keep current scene
pauseOptions.bPauseCurrent = true;    // Pause gameplay
GetSceneManager().LoadSceneAdditive("PauseMenu", pauseOptions);

// Preload next level
GetSceneManager().PreloadScene("Level2");
```

---

### 3. Scene (Runtime Scene Instance)

**Responsibilities:**
- Manage scene-specific game state
- Own a World (ECS container)
- Implement scene lifecycle hooks
- Handle scene layers for render/update ordering
- Provide template for instantiation

**Interface:**

```cpp
namespace Umbra {

    enum class ESceneState {
        Uninitialized,  // Just created
        Initializing,   // Initialize() running
        Ready,          // Initialized but not started
        Running,        // BeginPlay called, actively updating
        Paused,         // Update paused, still rendered
        Suspended,      // Update and render paused
        Ending,         // EndPlay() called
        Destroyed       // ShutDown() called
    };

    class Scene {
    public:
        Scene();
        virtual ~Scene();

        // Lifecycle hooks (implement in derived classes)

        /**
         * Called once when scene is instantiated.
         * Use for: System registration, camera setup, asset references
         */
        virtual void Initialize() = 0;

        /**
         * Called when scene becomes active and starts running.
         * Use for: Entity spawning, game state initialization
         */
        virtual void BeginPlay() = 0;

        /**
         * Called every frame while scene is active.
         * Use for: Input handling, game logic
         */
        virtual void Update(float _deltaTime) = 0;

        /**
         * Called at fixed timestep for physics/simulation.
         * Use for: Physics-dependent logic
         */
        virtual void FixedUpdate(float _fixedDelta) {}

        /**
         * Called when scene is about to be deactivated.
         * Use for: Cleanup, save state
         */
        virtual void EndPlay() {}

        /**
         * Called when scene is being destroyed.
         * Use for: Resource cleanup
         */
        virtual void Shutdown() {}

        /**
         * Create a copy of this scene for instantiation.
         * Default implementation uses copy constructor.
         */
        virtual UniquePtr<Scene> Instantiate() const;

        // Scene state
        ESceneState GetState() const { return mState; }
        bool IsRunning() const { return mState == ESceneState::Running; }
        bool IsPaused() const { return mState == ESceneState::Paused; }

        // Scene identity
        const String& GetSceneId() const { return mSceneId; }
        void SetSceneId(const String& _id) { mSceneId = _id; }

        // Scene context (data passed from previous scene)
        const SceneContext& GetSceneContext() const { return mContext; }

        // World access
        World* GetWorld() { return mWorld.get(); }
        const World* GetWorld() const { return mWorld.get(); }

        // Camera access
        EntityID GetMainCamera() const { return mMainCamera; }
        void SetMainCamera(EntityID _camera) { mMainCamera = _camera; }

        // Scene services
        SceneManager& GetSceneManager();
        IGameInstance& GetGameInstance();

        // Layer management
        void SetLayerUpdateOrder(const Vector<String>& _layerOrder);
        void SetLayerRenderOrder(const Vector<String>& _layerOrder);

    protected:
        // Optional: override for custom world setup
        virtual UniquePtr<World> CreateWorld();

        // Helper for common camera setup
        EntityID CreateDefaultCamera(float _orthoSize = 10.0f);

    private:
        friend class SceneManager;

        void InternalInitialize();
        void InternalBeginPlay();
        void InternalUpdate(float _deltaTime);
        void InternalFixedUpdate(float _fixedDelta);
        void InternalRender();
        void InternalEndPlay();
        void InternalShutdown();

        void SetGameInstance(IGameInstance* _gameInstance);
        void SetSceneManager(SceneManager* _sceneManager);
        void SetSceneContext(const SceneContext& _context);
        void TransitionState(ESceneState _newState);

        String mSceneId;
        ESceneState mState = ESceneState::Uninitialized;
        UniquePtr<World> mWorld;
        EntityID mMainCamera = MAX_ENTITY;
        SceneContext mContext;

        IGameInstance* mGameInstance = nullptr;
        SceneManager* mSceneManager = nullptr;

        Vector<String> mLayerUpdateOrder;
        Vector<String> mLayerRenderOrder;
    };

} // namespace Umbra
```

**Example Usage:**

```cpp
class Level1Scene : public Scene {
public:
    void Initialize() override {
        // Called once when scene template is created
        World* world = GetWorld();

        // Configure world systems
        world->AddSystem(ESystemPhase::Simulation, 10,
            MakeShared<EnemyAISystem>());
        world->AddSystem(ESystemPhase::Simulation, 20,
            MakeShared<ProjectileSystem>());
    }

    void BeginPlay() override {
        // Called when scene becomes active
        World* world = GetWorld();

        // Get scene context (data from previous scene)
        int playerHealth = GetSceneContext().GetInt("playerHealth", 100);
        int levelNumber = GetSceneContext().GetInt("levelNumber", 1);

        // Create camera
        mMainCamera = CreateDefaultCamera(15.0f);

        // Spawn entities
        SpawnPlayer(playerHealth);
        SpawnEnemies(levelNumber);
        SpawnPowerups();
    }

    void Update(float _deltaTime) override {
        // Handle input
        if (Input::GetKeyDown(KeyBoard::Escape)) {
            // Open pause menu
            SceneLoadOptions pauseOptions;
            pauseOptions.bUnloadCurrent = false;
            pauseOptions.bPauseCurrent = true;
            GetSceneManager().LoadSceneAdditive("PauseMenu", pauseOptions);
        }

        // Check win condition
        if (AllEnemiesDefeated()) {
            LoadNextLevel();
        }
    }

    void FixedUpdate(float _fixedDelta) override {
        // Physics-dependent logic
    }

    void EndPlay() override {
        // Save current state before leaving
        SaveCheckpoint();
    }

    UniquePtr<Scene> Instantiate() const override {
        return MakeUnique<Level1Scene>(*this);
    }

private:
    void SpawnPlayer(int _health) { /* ... */ }
    void SpawnEnemies(int _count) { /* ... */ }
    void SpawnPowerups() { /* ... */ }
    bool AllEnemiesDefeated() { return false; }
    void LoadNextLevel() { /* ... */ }
    void SaveCheckpoint() { /* ... */ }

    EntityID mMainCamera = MAX_ENTITY;
    EntityID mPlayerEntity = MAX_ENTITY;
};
```

---

### 4. World (ECS Container)

**Responsibilities:**
- Manage ECS registry (entities, components, systems)
- Execute system updates in phases
- Provide entity/component API
- Handle coordinate space conversions
- Support queries and filtering

**Improvements:**
- Make system registration more flexible
- Remove hardcoded core systems (let Scene configure)
- Add query API for complex entity searches
- Support for entity templates (prefabs)

**Interface:**

```cpp
namespace Umbra {

    struct WorldConfig {
        bool bCreateRenderSystem = true;
        bool bCreatePhysicsSystem = true;
        bool bCreateCameraSystem = true;
        uint32 maxEntities = 10000;
    };

    class World {
    public:
        World(const WorldConfig& _config = {});
        ~World();

        // Entity management
        EntityID CreateEntity();
        void DestroyEntity(EntityID _entity);
        bool IsEntityValid(EntityID _entity) const;

        // Component management
        template <typename T>
        void AddComponent(EntityID _entity, const T& _component);

        template <typename T, typename... Args>
        void AddComponent(EntityID _entity, Args&&... _args);

        template <typename T>
        void RemoveComponent(EntityID _entity);

        template <typename T>
        bool HasComponent(EntityID _entity) const;

        template <typename T>
        T* GetComponent(EntityID _entity);

        template <typename T>
        const T* GetComponent(EntityID _entity) const;

        // System management
        void AddSystem(ESystemPhase _phase, int _priority,
                       SharedPtr<System> _system);
        void RemoveSystem(SharedPtr<System>& _system);

        template <typename T>
        T* GetSystem();

        // Tag management
        void AddTag(EntityID _entity, const String& _tag);
        void RemoveTag(EntityID _entity, const String& _tag);
        bool HasTag(EntityID _entity, const String& _tag) const;
        Vector<EntityID> FindEntitiesByTag(const String& _tag) const;
        const Set<EntityID>& GetEntitiesByTag(const String& _tag) const;

        // Entity queries
        template <typename... Components>
        Vector<EntityID> FindEntitiesWith();

        // Prefab/template system
        EntityID InstantiatePrefab(const String& _prefabId);
        void RegisterPrefab(const String& _prefabId,
                            const EntityTemplate& _template);

        // Coordinate space conversion
        Math::Vector2f ScreenToWorld(const Math::Vector2i& _screenPos) const;
        Math::Vector2i WorldToScreen(const Math::Vector2f& _worldPos) const;

        // Update
        void Update(float _deltaTime);
        void FixedUpdate(float _fixedDelta);
        void Render();

        // Direct system access (optional, for advanced use)
        PhysicsSystem* GetPhysicsSystem();
        RenderSystem* GetRenderSystem();
        CameraSystem* GetCameraSystem();

    private:
        void InitializeCoreSystemsIfNeeded(const WorldConfig& _config);

        SharedPtr<ECSRegister> mWorldRegister;

        // Core systems (optional based on config)
        SharedPtr<RenderSystem> mRenderSystem;
        SharedPtr<PhysicsSystem> mPhysicsSystem;
        SharedPtr<CameraSystem> mCameraSystem;

        // Prefab registry
        UMap<String, EntityTemplate> mPrefabs;
    };

} // namespace Umbra

#include "Game/World.inl"  // Template implementations
```

**Example Usage:**

```cpp
// Create world with default systems
World* world = scene->GetWorld();

// Create entity with components
EntityID enemy = world->CreateEntity();
world->AddComponent<TransformComponent>(enemy, position, rotation, scale);
world->AddComponent<SpriteComponent>(enemy, spriteTexture, Color::Red);
world->AddComponent<PhysicsBodyComponent>(enemy, mass, drag);
world->AddComponent<EnemyAIComponent>(enemy, aggroRange, speed);
world->AddTag(enemy, "Enemy");

// Find all enemies
Vector<EntityID> enemies = world->FindEntitiesByTag("Enemy");

// Query entities with specific components
auto movableEntities = world->FindEntitiesWith<TransformComponent,
                                                PhysicsBodyComponent>();

// Use prefabs
world->RegisterPrefab("Grunt", CreateGruntTemplate());
EntityID grunt1 = world->InstantiatePrefab("Grunt");
EntityID grunt2 = world->InstantiatePrefab("Grunt");
```

---

## Advanced Features

### 1. Entity Templates / Prefabs

**Purpose:** Reusable entity configurations

**Implementation:**

```cpp
namespace Umbra {

    struct ComponentData {
        String componentType;  // RTTI or string ID
        Vector<byte> data;     // Serialized component data
    };

    struct EntityTemplate {
        Vector<ComponentData> components;
        Vector<String> tags;
        String name;
    };

    class PrefabManager {
    public:
        void RegisterPrefab(const String& _id, const EntityTemplate& _template);
        EntityID InstantiatePrefab(World* _world, const String& _id);

        // Serialization
        bool SavePrefab(const String& _id, const String& _filePath);
        bool LoadPrefab(const String& _filePath);

    private:
        UMap<String, EntityTemplate> mPrefabs;
    };

} // namespace Umbra
```

**Usage:**

```cpp
// Define prefab programmatically
EntityTemplate playerTemplate;
playerTemplate.name = "Player";
playerTemplate.components.push_back(
    SerializeComponent(TransformComponent{...}));
playerTemplate.components.push_back(
    SerializeComponent(SpriteComponent{...}));
playerTemplate.tags = {"Player", "Controllable"};

world->RegisterPrefab("Player", playerTemplate);

// Instantiate multiple copies
EntityID player1 = world->InstantiatePrefab("Player");
EntityID player2 = world->InstantiatePrefab("Player");
```

---

### 2. Scene Layers

**Purpose:** Control update and render order within scenes

**Implementation:**

```cpp
namespace Umbra {

    class SceneLayer {
    public:
        SceneLayer(const String& _name, int _priority);

        const String& GetName() const { return mName; }
        int GetPriority() const { return mPriority; }

        void AddEntity(EntityID _entity);
        void RemoveEntity(EntityID _entity);

        const Set<EntityID>& GetEntities() const { return mEntities; }

    private:
        String mName;
        int mPriority;
        Set<EntityID> mEntities;
    };

    class LayerManager {
    public:
        void CreateLayer(const String& _name, int _priority);
        void RemoveLayer(const String& _name);

        SceneLayer* GetLayer(const String& _name);
        Vector<SceneLayer*> GetLayersSortedByPriority();

        void AssignEntityToLayer(EntityID _entity, const String& _layer);

    private:
        UMap<String, UniquePtr<SceneLayer>> mLayers;
    };

} // namespace Umbra
```

**Usage:**

```cpp
// In Scene::Initialize()
LayerManager& layers = GetWorld()->GetLayerManager();
layers.CreateLayer("Background", 0);
layers.CreateLayer("Gameplay", 10);
layers.CreateLayer("UI", 100);

// Assign entities
EntityID background = world->CreateEntity();
layers.AssignEntityToLayer(background, "Background");

EntityID player = world->CreateEntity();
layers.AssignEntityToLayer(player, "Gameplay");
```

---

### 3. Game State Persistence

**Purpose:** Save and load game state

**Implementation:**

```cpp
namespace Umbra {

    struct SaveData {
        String sceneName;
        SceneContext sceneContext;
        Vector<EntityTemplate> entities;
        UMap<String, String> gameVariables;
    };

    class SaveSystem {
    public:
        bool SaveGame(const String& _slotName, const SaveData& _data);
        bool LoadGame(const String& _slotName, SaveData& _outData);
        bool HasSave(const String& _slotName) const;
        void DeleteSave(const String& _slotName);

        Vector<String> GetAllSaveSlots() const;

    private:
        String GetSavePath(const String& _slotName) const;
        bool SerializeSaveData(const SaveData& _data, const String& _path);
        bool DeserializeSaveData(const String& _path, SaveData& _outData);
    };

} // namespace Umbra
```

**Usage:**

```cpp
// Save game
SaveData saveData;
saveData.sceneName = GetCurrentScene()->GetSceneId();
saveData.sceneContext.SetInt("playerHealth", playerHealth);
saveData.sceneContext.SetInt("levelNumber", currentLevel);
// ... serialize entities ...

SaveSystem saveSystem;
saveSystem.SaveGame("Slot1", saveData);

// Load game
SaveData loadedData;
if (saveSystem.LoadGame("Slot1", loadedData)) {
    SceneLoadOptions options;
    options.context = loadedData.sceneContext;
    GetSceneManager().LoadScene(loadedData.sceneName, options);
}
```

---

### 4. Asset Streaming and Preloading

**Purpose:** Load assets asynchronously to avoid frame hitches

**Implementation:**

```cpp
namespace Umbra {

    class AssetLoadRequest {
    public:
        String assetPath;
        EAssetType type;
        bool bLoadAsync = true;

        // Callback when loaded
        Function<void(SharedPtr<IResource>)> onComplete;
    };

    class AssetStreamingManager {
    public:
        void RequestAssetLoad(const AssetLoadRequest& _request);
        void CancelAssetLoad(const String& _assetPath);

        bool IsAssetLoaded(const String& _assetPath) const;
        float GetLoadProgress(const String& _assetPath) const;

        void Update();  // Process async loads

    private:
        struct LoadTask {
            AssetLoadRequest request;
            std::future<SharedPtr<IResource>> future;
            float progress;
        };

        UMap<String, LoadTask> mPendingLoads;
    };

} // namespace Umbra
```

---

## API Examples

### Complete Game Example

```cpp
// MyGame.h
class MyGame : public IGameInstance {
public:
    FGameConfig LoadGameConfig() override;
    void Initialize() override;
    void Shutdown() override;

private:
    int mPlayerScore = 0;
};

// MyGame.cpp
FGameConfig MyGame::LoadGameConfig() {
    FGameConfig config;
    config.windowTitle = "My 2D Game";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.fixedDeltaTime = 1.0f / 60.0f;
    return config;
}

void MyGame::Initialize() {
    SceneManager& sceneManager = GetSceneManager();

    // Register all scenes
    sceneManager.RegisterSceneAsset("MainMenu",
        MakeUnique<MainMenuScene>());
    sceneManager.RegisterSceneAsset("Level1",
        MakeUnique<Level1Scene>());
    sceneManager.RegisterSceneAsset("Level2",
        MakeUnique<Level2Scene>());
    sceneManager.RegisterSceneAsset("GameOver",
        MakeUnique<GameOverScene>());

    // Preload level 1
    sceneManager.PreloadScene("Level1");

    // Start at main menu
    sceneManager.LoadScene("MainMenu");
}

void MyGame::Shutdown() {
    UMBRA_LOG_INFO("Shutting down game. Final score: %d", mPlayerScore);
}

// Entry point
UniquePtr<IGameInstance> CreateApplication() {
    return MakeUnique<MyGame>();
}

// MainMenuScene.h
class MainMenuScene : public Scene {
public:
    void Initialize() override;
    void BeginPlay() override;
    void Update(float _deltaTime) override;
    UniquePtr<Scene> Instantiate() const override;

private:
    void OnPlayButtonClicked();
    void OnQuitButtonClicked();
};

// MainMenuScene.cpp
void MainMenuScene::Initialize() {
    // Setup is done once when template is created
}

void MainMenuScene::BeginPlay() {
    // Create UI elements
    World* world = GetWorld();

    EntityID titleText = world->CreateEntity();
    world->AddComponent<TransformComponent>(titleText,
        Math::Vector2f(0, 100), 0, Math::Vector2f(1, 1));
    world->AddComponent<TextComponent>(titleText,
        "MY AWESOME GAME", 48, Color::White);
}

void MainMenuScene::Update(float _deltaTime) {
    // Simple button handling with ImGui
    ImGui::SetNextWindowPos(ImVec2(400, 300));
    ImGui::Begin("Main Menu", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    if (ImGui::Button("Play", ImVec2(200, 50))) {
        OnPlayButtonClicked();
    }
    if (ImGui::Button("Quit", ImVec2(200, 50))) {
        OnQuitButtonClicked();
    }

    ImGui::End();
}

void MainMenuScene::OnPlayButtonClicked() {
    SceneLoadOptions options;
    options.transition = ESceneTransition::FadeInOut;
    options.transitionDuration = 1.0f;
    GetSceneManager().LoadScene("Level1", options);
}

void MainMenuScene::OnQuitButtonClicked() {
    GetGameInstance().RequestExit();
}

UniquePtr<Scene> MainMenuScene::Instantiate() const {
    return MakeUnique<MainMenuScene>(*this);
}

// Level1Scene.h
class Level1Scene : public Scene {
public:
    void Initialize() override;
    void BeginPlay() override;
    void Update(float _deltaTime) override;
    void FixedUpdate(float _fixedDelta) override;
    void EndPlay() override;
    UniquePtr<Scene> Instantiate() const override;

private:
    void SpawnPlayer();
    void SpawnEnemies();
    void CheckWinCondition();

    EntityID mPlayer = MAX_ENTITY;
    int mEnemiesRemaining = 0;
};

// Level1Scene.cpp
void Level1Scene::Initialize() {
    World* world = GetWorld();

    // Add custom systems
    world->AddSystem(ESystemPhase::Simulation, 10,
        MakeShared<PlayerControllerSystem>());
    world->AddSystem(ESystemPhase::Simulation, 20,
        MakeShared<EnemyAISystem>());
    world->AddSystem(ESystemPhase::Simulation, 30,
        MakeShared<BulletSystem>());
}

void Level1Scene::BeginPlay() {
    World* world = GetWorld();

    // Create camera
    EntityID camera = CreateDefaultCamera(20.0f);
    SetMainCamera(camera);

    // Spawn game entities
    SpawnPlayer();
    SpawnEnemies();
}

void Level1Scene::Update(float _deltaTime) {
    // Handle pause
    if (Input::GetKeyDown(KeyBoard::Escape)) {
        SceneLoadOptions pauseOptions;
        pauseOptions.bUnloadCurrent = false;
        pauseOptions.bPauseCurrent = true;
        GetSceneManager().LoadSceneAdditive("PauseMenu", pauseOptions);
    }

    CheckWinCondition();
}

void Level1Scene::FixedUpdate(float _fixedDelta) {
    // Physics-based logic here
}

void Level1Scene::EndPlay() {
    // Cleanup
    UMBRA_LOG_INFO("Level 1 completed!");
}

void Level1Scene::SpawnPlayer() {
    World* world = GetWorld();

    mPlayer = world->CreateEntity();
    world->AddComponent<TransformComponent>(mPlayer,
        Math::Vector2f(0, 0), 0, Math::Vector2f(1, 1));
    world->AddComponent<SpriteComponent>(mPlayer,
        "Assets/player.png", Color::White, Math::Vector2f(32, 32));
    world->AddComponent<PhysicsBodyComponent>(mPlayer,
        1.0f, 0.5f, EBodyType::Dynamic);
    world->AddComponent<BoxColliderComponent>(mPlayer,
        Math::Vector2f(32, 32));
    world->AddComponent<PlayerControllerComponent>(mPlayer,
        10.0f, 15.0f);  // speed, jump force
    world->AddTag(mPlayer, "Player");
}

void Level1Scene::SpawnEnemies() {
    World* world = GetWorld();

    // Spawn 5 enemies using prefab
    for (int i = 0; i < 5; ++i) {
        EntityID enemy = world->InstantiatePrefab("BasicEnemy");
        auto* transform = world->GetComponent<TransformComponent>(enemy);
        transform->position = Math::Vector2f(i * 5.0f, 0);
        mEnemiesRemaining++;
    }
}

void Level1Scene::CheckWinCondition() {
    World* world = GetWorld();
    Vector<EntityID> enemies = world->FindEntitiesByTag("Enemy");

    if (enemies.empty() && mEnemiesRemaining > 0) {
        // All enemies defeated, go to next level
        SceneLoadOptions options;
        options.transition = ESceneTransition::FadeInOut;
        options.context.SetInt("levelCompleted", 1);
        GetSceneManager().LoadScene("Level2", options);
    }
}

UniquePtr<Scene> Level1Scene::Instantiate() const {
    return MakeUnique<Level1Scene>(*this);
}
```

---

## Migration Guide

### From Current to New System

**Step 1: Update IGameInstance**

Old:
```cpp
FGameConfig& IGameInstance::LoadGameConfig() {
    return FGameConfig();  // BUG: returns reference to temporary
}
```

New:
```cpp
FGameConfig MyGame::LoadGameConfig() {
    FGameConfig config;
    config.windowTitle = "My Game";
    return config;  // Returns by value
}
```

**Step 2: Rename Lifecycle Methods**

Old:
```cpp
void OnBeginPlay() override;
void OnUpdate() override;
void OnFixedUpdated() override;  // Typo
void OnEndPlay() override;  // Never called
SharedPtr<Scene> InsatiateCopy() override;  // Typo
```

New:
```cpp
void BeginPlay() override;
void Update(float _deltaTime) override;
void FixedUpdate(float _fixedDelta) override;
void EndPlay() override;  // Now properly called
UniquePtr<Scene> Instantiate() const override;
```

**Step 3: Update Scene Transitions**

Old:
```cpp
GetSceneManager().GoToScene("Level1");
```

New:
```cpp
GetSceneManager().LoadScene("Level1");

// With options
SceneLoadOptions options;
options.context.SetInt("playerHealth", 100);
GetSceneManager().LoadScene("Level1", options);
```

**Step 4: Fix World::RemoveComponent**

Already fixed in [World.inl:54-56](Engine/Include/Game/World.inl#L54-L56):
```cpp
template <typename T>
inline void World::RemoveComponent(EntityID _entity) {
    mWorldRegister->RemoveComponent<T>(_entity);  // Fixed!
}
```

---

## Implementation Roadmap

### Phase 1: Critical Fixes (Week 1)
- [x] Fix World::RemoveComponent bug (already done)
- [ ] Fix LoadGameConfig return type
- [ ] Ensure OnEndPlay is called during scene transitions
- [ ] Fix typos: InsatiateCopy → Instantiate, OnFixedUpdated → FixedUpdate
- [ ] Remove commented dead code in IGameInstance.h

### Phase 2: Core Refactoring (Week 2-3)
- [ ] Clarify ownership model (Unique vs Shared pointers)
- [ ] Simplify Scene lifecycle (merge Initialize/BeginPlay or clarify)
- [ ] Remove UI coupling from Scene::Render
- [ ] Make camera creation optional in Scene
- [ ] Add SceneContext for scene data passing
- [ ] Implement proper EndPlay call in SceneManager

### Phase 3: New Features (Week 4-5)
- [ ] Implement SceneLoadOptions with transitions
- [ ] Add scene preloading system
- [ ] Add scene stack for overlays (pause menus)
- [ ] Implement ESceneState enum and state management
- [ ] Add pause/resume scene functionality

### Phase 4: Advanced Features (Week 6-8)
- [ ] Prefab/EntityTemplate system
- [ ] Scene layer system
- [ ] SaveSystem for game state persistence
- [ ] Asset streaming and async loading
- [ ] Query API for complex entity searches

### Phase 5: Polish & Documentation (Week 9-10)
- [ ] Complete API documentation
- [ ] Write usage examples and tutorials
- [ ] Performance profiling and optimization
- [ ] Update Architecture.md
- [ ] Create migration guide for existing projects

---

## Summary

This design provides a robust, extensible Game Framework layer that addresses all current issues while adding modern game engine features:

**Fixed Issues:**
- RemoveComponent bug
- LoadGameConfig undefined behavior
- Missing OnEndPlay calls
- Naming inconsistencies
- Ownership confusion

**New Features:**
- Scene state management (active/paused/suspended)
- Scene context for data passing
- Scene preloading and async loading
- Scene transitions with effects
- Prefab/template system
- Layer-based rendering
- Save/load system
- Scene stack for overlays

**Better API:**
- Clear lifecycle hooks
- Intuitive scene transitions
- Flexible system configuration
- Type-safe component access
- Comprehensive query system

The design maintains backward compatibility where possible and provides a clear migration path for existing code.
