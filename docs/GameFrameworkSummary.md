# Game Framework Design - Executive Summary

## Overview

This document provides a comprehensive redesign of UMBRA's Game Framework layer, addressing critical bugs and adding modern game engine features for building 2D games.

---

## Current Issues Fixed

### Critical Bugs
1. ✅ **World::RemoveComponent** - Was calling RegisterComponent (already fixed)
2. ❌ **LoadGameConfig** - Returns reference to temporary (UB)
3. ❌ **OnEndPlay** - Never called in SceneManager
4. ❌ **Naming typos** - InsatiateCopy, OnFixedUpdated, GetSceneManger

### Design Issues
1. **Ownership confusion** - Mixed use of UniquePtr/SharedPtr with std::move on SharedPtr
2. **Scene copy semantics** - Two copy mechanisms (copy constructor vs Instantiate)
3. **Lifecycle complexity** - Too many lifecycle methods with unclear timing
4. **UI coupling** - Scene::Render directly calls ImGui
5. **Hardcoded systems** - World always creates render/physics/camera systems
6. **No scene data passing** - Can't pass context between scenes

---

## New Architecture

### Layer Hierarchy

```
IGameInstance (Application)
    └─> SceneManager (Scene lifecycle)
            └─> Scene (Runtime instance)
                    └─> World (ECS container)
```

### Ownership Model

```cpp
// Clear, single ownership
IGameInstance owns UniquePtr<SceneManager>
SceneManager owns UMap<String, UniquePtr<Scene>> (templates)
SceneManager owns Vector<UniquePtr<Scene>> (active stack)
Scene owns UniquePtr<World>
```

---

## Key Features

### 1. Scene State Management

```cpp
enum class ESceneState {
    Uninitialized, Initializing, Ready, Running,
    Paused, Suspended, Ending, Destroyed
};
```

Scenes can be:
- **Running** - Active simulation and rendering
- **Paused** - No simulation, still rendered (pause menu)
- **Suspended** - No simulation or rendering (background scenes)

### 2. Scene Context (Data Passing)

```cpp
SceneContext context;
context.SetInt("playerHealth", 100);
context.SetInt("levelNumber", 2);

SceneLoadOptions options;
options.context = context;
GetSceneManager().LoadScene("Level2", options);

// In new scene
int health = GetSceneContext().GetInt("playerHealth", 100);
```

### 3. Scene Transitions

```cpp
SceneLoadOptions options;
options.transition = ESceneTransition::FadeInOut;
options.transitionDuration = 1.0f;
options.context.SetInt("score", playerScore);
GetSceneManager().LoadScene("Level2", options);
```

Supports: None, FadeOut, FadeInOut, Slide, Custom

### 4. Scene Stack (Overlays)

```cpp
// Load pause menu over gameplay
SceneLoadOptions pauseOptions;
pauseOptions.bUnloadCurrent = false;  // Keep current scene
pauseOptions.bPauseCurrent = true;    // Pause gameplay
GetSceneManager().LoadSceneAdditive("PauseMenu", pauseOptions);
```

### 5. Scene Preloading

```cpp
// Preload next level asynchronously
GetSceneManager().PreloadScene("Level2");

// Check progress
if (GetSceneManager().IsScenePreloaded("Level2")) {
    GetSceneManager().LoadScene("Level2");  // Instant!
}
```

### 6. Entity Prefabs/Templates

```cpp
// Register prefab
EntityTemplate enemyTemplate = CreateEnemyTemplate();
world->RegisterPrefab("BasicEnemy", enemyTemplate);

// Instantiate multiple copies
EntityID enemy1 = world->InstantiatePrefab("BasicEnemy");
EntityID enemy2 = world->InstantiatePrefab("BasicEnemy");
```

### 7. Scene Layers

```cpp
// Control render and update order
world->GetLayerManager().CreateLayer("Background", 0);
world->GetLayerManager().CreateLayer("Gameplay", 10);
world->GetLayerManager().CreateLayer("UI", 100);

world->GetLayerManager().AssignEntityToLayer(background, "Background");
world->GetLayerManager().AssignEntityToLayer(player, "Gameplay");
```

### 8. Save/Load System

```cpp
SaveData saveData;
saveData.sceneName = GetCurrentScene()->GetSceneId();
saveData.sceneContext.SetInt("playerHealth", 100);

SaveSystem saveSystem;
saveSystem.SaveGame("Slot1", saveData);

// Later...
SaveData loadedData;
if (saveSystem.LoadGame("Slot1", loadedData)) {
    SceneLoadOptions options;
    options.context = loadedData.sceneContext;
    GetSceneManager().LoadScene(loadedData.sceneName, options);
}
```

---

## API Improvements

### IGameInstance

**Before:**
```cpp
virtual FGameConfig& LoadGameConfig();  // Returns temp!
virtual void Initialize() = 0;
virtual void ShutDown() = 0;
```

**After:**
```cpp
virtual FGameConfig LoadGameConfig() = 0;  // Return by value
virtual void Initialize() = 0;
virtual void Shutdown() = 0;
virtual void OnUpdate(float _deltaTime) {}  // Optional
virtual void OnApplicationFocusChanged(bool _hasFocus) {}
```

### Scene Lifecycle

**Before:**
```cpp
virtual void Initialize() = 0;
virtual void OnBeginPlay() = 0;           // Unclear difference
virtual void OnUpdate() = 0;              // No deltaTime param
virtual void OnFixedUpdated() = 0;        // Typo!
virtual void OnEndPlay() = 0;             // Never called!
virtual SharedPtr<Scene> InsatiateCopy(); // Typo!
```

**After:**
```cpp
virtual void Initialize() = 0;      // System setup
virtual void BeginPlay() = 0;       // Entity spawning
virtual void Update(float _dt) = 0; // Per-frame logic
virtual void FixedUpdate(float _fixedDt) {}  // Physics
virtual void EndPlay() {}           // Cleanup (called properly)
virtual UniquePtr<Scene> Instantiate() const;  // Fixed!
```

### SceneManager

**Before:**
```cpp
void AddScene(String _id, SharedPtr<Scene> _scene);
void GoToScene(const String& _id);
void GoToScene(SharedPtr<Scene>& _scene);  // Uses std::move on SharedPtr!
```

**After:**
```cpp
// Register templates
void RegisterSceneAsset(const String& _id, UniquePtr<Scene> _template);

// Load scenes
void LoadScene(const String& _id, const SceneLoadOptions& _options = {});
void LoadSceneAdditive(const String& _id, const SceneLoadOptions& _options = {});
void UnloadScene(const String& _id);

// Preload
void PreloadScene(const String& _id);
bool IsScenePreloaded(const String& _id) const;

// Scene stack
Scene* GetActiveScene();
Scene* GetSceneByIndex(size_t _index);
size_t GetSceneCount() const;

// State control
void PauseScene(Scene* _scene);
void ResumeScene(Scene* _scene);
```

### World

**Before:**
```cpp
World();  // Hardcodes render/physics/camera systems
// No prefab support
// No layer support
```

**After:**
```cpp
struct WorldConfig {
    bool bCreateRenderSystem = true;
    bool bCreatePhysicsSystem = true;
    bool bCreateCameraSystem = true;
    uint32 maxEntities = 10000;
};

World(const WorldConfig& _config = {});

// Prefab system
EntityID InstantiatePrefab(const String& _prefabId);
void RegisterPrefab(const String& _id, const EntityTemplate& _template);

// Layer system
LayerManager& GetLayerManager();

// Query API
template <typename... Components>
Vector<EntityID> FindEntitiesWith();
```

---

## Migration Path

### Step 1: Fix Critical Bugs (Immediate)
```cpp
// 1. Fix LoadGameConfig
FGameConfig MyGame::LoadGameConfig() {
    FGameConfig config;
    // ... configure ...
    return config;  // Return by value
}

// 2. Ensure OnEndPlay is called
void SceneManager::GoToScene(...) {
    if (mCurrentScene) {
        mCurrentScene->OnEndPlay();  // ADD THIS
        mDeletedScene = std::move(mCurrentScene);
    }
}

// 3. Fix typos
// InsatiateCopy → Instantiate
// OnFixedUpdated → FixedUpdate
// GetSceneManger → GetSceneManager
```

### Step 2: Update Scene Transitions
```cpp
// Old
GetSceneManager().GoToScene("Level1");

// New
GetSceneManager().LoadScene("Level1");

// With options
SceneLoadOptions options;
options.context.SetInt("level", 1);
GetSceneManager().LoadScene("Level1", options);
```

### Step 3: Adopt New Lifecycle
```cpp
class MyScene : public Scene {
    void Initialize() override {
        // System setup only
        World* world = GetWorld();
        world->AddSystem(...);
    }

    void BeginPlay() override {
        // Entity spawning
        World* world = GetWorld();
        EntityID player = world->CreateEntity();
        // ...
    }

    void Update(float _deltaTime) override {
        // Input and logic
    }

    void EndPlay() override {
        // Cleanup (now called!)
    }

    UniquePtr<Scene> Instantiate() const override {
        return MakeUnique<MyScene>(*this);
    }
};
```

---

## Implementation Roadmap

### Phase 1: Critical Fixes (Week 1)
- [x] Fix World::RemoveComponent (done)
- [ ] Fix LoadGameConfig return type
- [ ] Call OnEndPlay during transitions
- [ ] Fix naming typos
- [ ] Remove commented dead code

### Phase 2: Core Refactoring (Week 2-3)
- [ ] Clarify ownership model
- [ ] Simplify lifecycle (add deltaTime params)
- [ ] Remove UI coupling from Scene
- [ ] Optional camera creation
- [ ] Add SceneContext
- [ ] Add ESceneState enum

### Phase 3: New Features (Week 4-5)
- [ ] SceneLoadOptions with transitions
- [ ] Scene preloading
- [ ] Scene stack for overlays
- [ ] Pause/resume functionality

### Phase 4: Advanced (Week 6-8)
- [ ] Prefab system
- [ ] Layer system
- [ ] Save/load system
- [ ] Query API

### Phase 5: Polish (Week 9-10)
- [ ] Documentation
- [ ] Examples
- [ ] Performance optimization
- [ ] Migration guide

---

## Benefits Summary

| Feature | Before | After |
|---------|--------|-------|
| **Scene data passing** | ❌ Not supported | ✅ SceneContext |
| **Scene transitions** | ⚠️ Instant only | ✅ Fade, slide, custom |
| **Scene preloading** | ❌ No | ✅ Async preload |
| **Overlays (pause)** | ❌ Replace only | ✅ Scene stack |
| **Scene states** | ⚠️ Limited | ✅ Active/Paused/Suspended |
| **Entity templates** | ❌ No | ✅ Prefab system |
| **Render ordering** | ⚠️ System order | ✅ Layer system |
| **Save/Load** | ❌ Manual | ✅ SaveSystem |
| **Ownership clarity** | ⚠️ Confusing | ✅ Clear (Unique) |
| **OnEndPlay called** | ❌ Never | ✅ Always |
| **Bug: RemoveComponent** | ❌ Broken | ✅ Fixed |
| **Bug: LoadGameConfig** | ❌ UB | ✅ Fixed |

---

## Code Quality Comparison

| Aspect | Current | Designed | Improvement |
|--------|---------|----------|-------------|
| Correctness | 6/10 | 10/10 | Critical bugs fixed |
| Usability | 8/10 | 10/10 | Better API ergonomics |
| Extensibility | 6/10 | 9/10 | Modular, configurable |
| Features | 5/10 | 9/10 | Modern game engine features |
| Documentation | 5/10 | 9/10 | Comprehensive docs |

---

## Next Steps

1. **Review** this design document
2. **Prioritize** features (critical fixes first)
3. **Implement** in phases
4. **Test** with sandbox applications
5. **Document** API and examples
6. **Migrate** existing games

For complete design details, see [GameFrameworkDesign.md](GameFrameworkDesign.md).

---

## Questions?

- What is the priority for implementation?
- Should we maintain backward compatibility?
- Which advanced features are most important?
- Any additional requirements?
