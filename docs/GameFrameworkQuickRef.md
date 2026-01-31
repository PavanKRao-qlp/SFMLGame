# Game Framework - Quick Reference

A concise reference for the UMBRA Game Framework API.

---

## Creating a Game

### 1. Define Your Game Instance

```cpp
// MyGame.h
#pragma once
#include "Game/IGameInstance.h"

class MyGame : public Umbra::IGameInstance {
public:
    Umbra::FGameConfig LoadGameConfig() override;
    void Initialize() override;
    void Shutdown() override;
    void OnUpdate(float _deltaTime) override;
};

// MyGame.cpp
#include "MyGame.h"

FGameConfig MyGame::LoadGameConfig() {
    FGameConfig config;
    config.windowTitle = "My Awesome Game";
    config.windowWidth = 1920;
    config.windowHeight = 1080;
    config.fixedDeltaTime = 1.0f / 60.0f;
    return config;
}

void MyGame::Initialize() {
    // Register scene templates
    GetSceneManager().RegisterSceneAsset("MainMenu",
        MakeUnique<MainMenuScene>());
    GetSceneManager().RegisterSceneAsset("Level1",
        MakeUnique<Level1Scene>());

    // Start at main menu
    GetSceneManager().LoadScene("MainMenu");
}

void MyGame::Shutdown() {
    // Cleanup
}

void MyGame::OnUpdate(float _deltaTime) {
    // Global input (F11 for fullscreen, etc.)
}

// Entry point
UniquePtr<IGameInstance> CreateApplication() {
    return MakeUnique<MyGame>();
}
```

---

## Creating Scenes

### 2. Define Your Scene

```cpp
// Level1Scene.h
#pragma once
#include "Game/Scene.h"

class Level1Scene : public Umbra::Scene {
public:
    void Initialize() override;
    void BeginPlay() override;
    void Update(float _deltaTime) override;
    void FixedUpdate(float _fixedDelta) override;
    void EndPlay() override;
    UniquePtr<Scene> Instantiate() const override;

private:
    EntityID mPlayer = MAX_ENTITY;
};

// Level1Scene.cpp
#include "Level1Scene.h"

void Level1Scene::Initialize() {
    // Called once: Setup systems
    World* world = GetWorld();
    world->AddSystem(ESystemPhase::Simulation, 10,
        MakeShared<PlayerControllerSystem>());
}

void Level1Scene::BeginPlay() {
    // Called when scene starts: Spawn entities
    World* world = GetWorld();

    // Create camera
    EntityID camera = CreateDefaultCamera(20.0f);
    SetMainCamera(camera);

    // Get context from previous scene
    int playerHealth = GetSceneContext().GetInt("playerHealth", 100);

    // Spawn player
    mPlayer = world->CreateEntity();
    world->AddComponent<TransformComponent>(mPlayer,
        Math::Vector2f(0, 0), 0, Math::Vector2f(1, 1));
    world->AddComponent<SpriteComponent>(mPlayer,
        "player.png", Color::White, Math::Vector2f(32, 32));
    world->AddTag(mPlayer, "Player");
}

void Level1Scene::Update(float _deltaTime) {
    // Per-frame logic
    if (Input::GetKeyDown(KeyBoard::Escape)) {
        // Open pause menu
        SceneLoadOptions opts;
        opts.bUnloadCurrent = false;
        opts.bPauseCurrent = true;
        GetSceneManager().LoadSceneAdditive("PauseMenu", opts);
    }
}

void Level1Scene::FixedUpdate(float _fixedDelta) {
    // Physics-dependent logic
}

void Level1Scene::EndPlay() {
    // Cleanup before leaving scene
    UMBRA_LOG_INFO("Level1 ended");
}

UniquePtr<Scene> Level1Scene::Instantiate() const {
    return MakeUnique<Level1Scene>(*this);
}
```

---

## Scene Transitions

### Load Scene (Replace Current)

```cpp
// Simple
GetSceneManager().LoadScene("Level1");

// With transition
SceneLoadOptions options;
options.transition = ESceneTransition::FadeInOut;
options.transitionDuration = 1.0f;
GetSceneManager().LoadScene("Level1", options);

// With context data
SceneLoadOptions options;
options.context.SetInt("playerHealth", 85);
options.context.SetInt("levelNumber", 2);
GetSceneManager().LoadScene("Level2", options);
```

### Load Additive (Overlay)

```cpp
// Pause menu over gameplay
SceneLoadOptions pauseOptions;
pauseOptions.bUnloadCurrent = false;  // Keep current
pauseOptions.bPauseCurrent = true;    // Pause current
GetSceneManager().LoadSceneAdditive("PauseMenu", pauseOptions);

// Close overlay
GetSceneManager().UnloadScene("PauseMenu");
// Previous scene automatically resumes
```

### Preload Scenes

```cpp
// Start preloading
GetSceneManager().PreloadScene("Level2");

// Check if ready
if (GetSceneManager().IsScenePreloaded("Level2")) {
    GetSceneManager().LoadScene("Level2");  // Instant switch!
}

// Check progress
float progress = GetSceneManager().GetPreloadProgress("Level2");
DisplayLoadingBar(progress);  // 0.0 to 1.0
```

---

## Entity Management

### Create Entities

```cpp
World* world = GetWorld();

// Create entity
EntityID entity = world->CreateEntity();

// Add components
world->AddComponent<TransformComponent>(entity,
    Math::Vector2f(0, 0),           // position
    0.0f,                           // rotation
    Math::Vector2f(1, 1));          // scale

world->AddComponent<SpriteComponent>(entity,
    "sprite.png",                   // texture path
    Color::White,                   // color
    Math::Vector2f(32, 32));        // size

world->AddComponent<PhysicsBodyComponent>(entity,
    1.0f,                           // mass
    0.5f,                           // drag
    EBodyType::Dynamic);

// Add tags
world->AddTag(entity, "Enemy");

// Destroy entity
world->DestroyEntity(entity);
```

### Query Entities

```cpp
// By tag
Vector<EntityID> enemies = world->FindEntitiesByTag("Enemy");
const Set<EntityID>& players = world->GetEntitiesByTag("Player");

// By component (planned)
auto entities = world->FindEntitiesWith<TransformComponent,
                                         PhysicsBodyComponent>();

// Check tags
if (world->HasTag(entity, "Player")) {
    // ...
}
```

### Modify Components

```cpp
// Get component
auto* transform = world->GetComponent<TransformComponent>(entity);
if (transform) {
    transform->position.x += 10.0f;
}

// Check if entity has component
if (world->HasComponent<PhysicsBodyComponent>(entity)) {
    // ...
}

// Remove component
world->RemoveComponent<SpriteComponent>(entity);
```

---

## Prefab System (Planned)

### Register Prefab

```cpp
EntityTemplate CreateEnemyPrefab() {
    EntityTemplate templ;
    templ.name = "BasicEnemy";
    templ.components = {
        SerializeComponent(TransformComponent{...}),
        SerializeComponent(SpriteComponent{...}),
        SerializeComponent(PhysicsBodyComponent{...}),
        SerializeComponent<EnemyAIComponent>({...})
    };
    templ.tags = {"Enemy", "Hostile"};
    return templ;
}

void MyScene::Initialize() {
    World* world = GetWorld();
    world->RegisterPrefab("BasicEnemy", CreateEnemyPrefab());
}
```

### Instantiate Prefab

```cpp
void MyScene::SpawnWave() {
    World* world = GetWorld();

    for (int i = 0; i < 10; ++i) {
        EntityID enemy = world->InstantiatePrefab("BasicEnemy");

        // Customize instance
        auto* transform = world->GetComponent<TransformComponent>(enemy);
        transform->position = GetRandomSpawnPoint();
    }
}
```

---

## Input Handling

```cpp
void MyScene::Update(float _deltaTime) {
    using namespace Umbra;

    // Keyboard
    if (Input::GetKeyDown(KeyBoard::Space)) {
        Jump();
    }

    if (Input::GetKey(KeyBoard::W)) {
        MoveForward(_deltaTime);
    }

    if (Input::GetKeyUp(KeyBoard::LShift)) {
        StopSprinting();
    }

    // Mouse
    if (Input::GetMouseButtonDown(Mouse::Left)) {
        Shoot();
    }

    Math::Vector2i mousePos = Input::GetMousePosition();
    Math::Vector2f worldPos = GetWorld()->ScreenToWorld(mousePos);
}
```

---

## Camera Management

### Create Camera

```cpp
void MyScene::BeginPlay() {
    // Option 1: Use helper
    EntityID camera = CreateDefaultCamera(15.0f);  // Ortho size
    SetMainCamera(camera);

    // Option 2: Manual
    EntityID camera = GetWorld()->CreateEntity();
    GetWorld()->AddComponent<TransformComponent>(camera,
        Math::Vector2f(0, 0), 0, Math::Vector2f(1, 1));
    GetWorld()->AddComponent<CameraComponent>(camera,
        15.0f,    // orthoSize
        0.1f,     // nearClip
        100.0f);  // farClip
    SetMainCamera(camera);
}
```

### Camera Movement

```cpp
void MyScene::Update(float _deltaTime) {
    EntityID camera = GetMainCamera();
    auto* transform = GetWorld()->GetComponent<TransformComponent>(camera);

    // Pan camera
    if (Input::GetKey(KeyBoard::Left)) {
        transform->position.x -= 10.0f * _deltaTime;
    }

    // Zoom (modify camera component)
    auto* cam = GetWorld()->GetComponent<CameraComponent>(camera);
    if (Input::GetKey(KeyBoard::Q)) {
        cam->orthographicSize *= 1.01f;  // Zoom out
    }
}
```

---

## System Management

### Add Custom System

```cpp
class MyCustomSystem : public System {
public:
    void Initialize(ECSRegister* _register) override {
        mView = _register->CreateView<TransformComponent, MyComponent>();
    }

    void Update(ECSRegister* _register) override {
        for (EntityID entity : mView->GetEntities()) {
            auto* transform = _register->GetComponent<TransformComponent>(entity);
            auto* myComp = _register->GetComponent<MyComponent>(entity);

            // Do work
            transform->position.x += myComp->velocity * EngineTime::GetDeltaTime();
        }
    }

private:
    ECView* mView = nullptr;
};

// In Scene::Initialize()
void MyScene::Initialize() {
    World* world = GetWorld();
    world->AddSystem(ESystemPhase::Simulation, 10,
        MakeShared<MyCustomSystem>());
}
```

### Access Core Systems

```cpp
World* world = GetWorld();

// Physics
PhysicsSystem* physics = world->GetPhysicsSystem();
if (physics) {
    physics->SetGravity(Math::Vector2f(0, -9.8f));
}

// Render
RenderSystem* render = world->GetRenderSystem();
// ...

// Camera
CameraSystem* camera = world->GetCameraSystem();
// ...
```

---

## Layer System (Planned)

```cpp
void MyScene::Initialize() {
    World* world = GetWorld();
    LayerManager& layers = world->GetLayerManager();

    // Create layers
    layers.CreateLayer("Background", 0);
    layers.CreateLayer("Gameplay", 10);
    layers.CreateLayer("Foreground", 50);
    layers.CreateLayer("UI", 100);
}

void MyScene::BeginPlay() {
    LayerManager& layers = GetWorld()->GetLayerManager();

    EntityID bg = CreateBackgroundEntity();
    layers.AssignEntityToLayer(bg, "Background");

    EntityID player = CreatePlayer();
    layers.AssignEntityToLayer(player, "Gameplay");

    EntityID hud = CreateHUD();
    layers.AssignEntityToLayer(hud, "UI");
}
```

---

## Save/Load (Planned)

### Save Game

```cpp
void MyScene::SaveCheckpoint() {
    SaveData saveData;

    // Save scene ID
    saveData.sceneName = GetSceneId();

    // Save game state
    saveData.sceneContext.SetInt("playerHealth", GetPlayerHealth());
    saveData.sceneContext.SetInt("playerScore", GetPlayerScore());
    saveData.sceneContext.SetInt("levelNumber", mLevelNumber);
    saveData.sceneContext.SetString("lastCheckpoint", mCheckpointId);

    // Save entity data (serialize world state)
    // ... implementation depends on serialization system ...

    SaveSystem saveSystem;
    if (saveSystem.SaveGame("Slot1", saveData)) {
        UMBRA_LOG_INFO("Game saved successfully");
    }
}
```

### Load Game

```cpp
void MainMenuScene::OnLoadButtonClicked() {
    SaveSystem saveSystem;
    SaveData loadedData;

    if (saveSystem.LoadGame("Slot1", loadedData)) {
        SceneLoadOptions options;
        options.context = loadedData.sceneContext;
        options.transition = ESceneTransition::FadeInOut;

        GetSceneManager().LoadScene(loadedData.sceneName, options);
    } else {
        UMBRA_LOG_ERROR("Failed to load save file");
    }
}
```

---

## Common Patterns

### Player Controller Pattern

```cpp
class GameplayScene : public Scene {
    void BeginPlay() override {
        World* world = GetWorld();

        mPlayer = world->CreateEntity();
        world->AddComponent<TransformComponent>(mPlayer, ...);
        world->AddComponent<SpriteComponent>(mPlayer, ...);
        world->AddComponent<PhysicsBodyComponent>(mPlayer, ...);
        world->AddComponent<PlayerControllerComponent>(mPlayer, ...);
        world->AddTag(mPlayer, "Player");
    }

    void Update(float _deltaTime) override {
        // PlayerControllerSystem handles movement
        // Check for game events here
        if (GetPlayerHealth() <= 0) {
            GetSceneManager().LoadScene("GameOver");
        }
    }

private:
    EntityID mPlayer = MAX_ENTITY;
};
```

### Pause Menu Pattern

```cpp
void GameplayScene::Update(float _deltaTime) {
    if (Input::GetKeyDown(KeyBoard::Escape)) {
        OpenPauseMenu();
    }
}

void GameplayScene::OpenPauseMenu() {
    SceneLoadOptions options;
    options.bUnloadCurrent = false;  // Keep this scene loaded
    options.bPauseCurrent = true;    // Pause updates
    GetSceneManager().LoadSceneAdditive("PauseMenu", options);
}

// In PauseMenuScene
void PauseMenuScene::OnResumeClicked() {
    GetSceneManager().UnloadScene("PauseMenu");
    // Previous scene automatically resumes
}
```

### Level Progression Pattern

```cpp
void Level1Scene::CheckWinCondition() {
    Vector<EntityID> enemies = GetWorld()->FindEntitiesByTag("Enemy");

    if (enemies.empty()) {
        // Level complete, go to next
        SceneLoadOptions options;
        options.transition = ESceneTransition::FadeInOut;
        options.context.SetInt("playerHealth", GetPlayerHealth());
        options.context.SetInt("playerScore", mScore);
        options.context.SetInt("levelNumber", 2);

        GetSceneManager().LoadScene("Level2", options);
    }
}
```

### Enemy Spawner Pattern

```cpp
class Level1Scene : public Scene {
    void Initialize() override {
        // Register enemy prefab
        World* world = GetWorld();
        world->RegisterPrefab("Grunt", CreateGruntPrefab());
    }

    void BeginPlay() override {
        SpawnEnemyWave(5);
    }

    void SpawnEnemyWave(int _count) {
        World* world = GetWorld();

        for (int i = 0; i < _count; ++i) {
            EntityID enemy = world->InstantiatePrefab("Grunt");

            // Randomize spawn position
            auto* transform = world->GetComponent<TransformComponent>(enemy);
            transform->position = GetRandomSpawnPoint();
        }
    }

private:
    EntityTemplate CreateGruntPrefab() {
        EntityTemplate templ;
        templ.name = "Grunt";
        // ... add components ...
        templ.tags = {"Enemy", "Grunt"};
        return templ;
    }

    Math::Vector2f GetRandomSpawnPoint() {
        // Random logic
        return Math::Vector2f(Random::Range(-50, 50),
                              Random::Range(-50, 50));
    }
};
```

---

## Coordinate Conversion

```cpp
// Screen to world
Math::Vector2i mouseScreen = Input::GetMousePosition();
Math::Vector2f mouseWorld = GetWorld()->ScreenToWorld(mouseScreen);

// World to screen
Math::Vector2f entityWorldPos = transform->position;
Math::Vector2i entityScreenPos = GetWorld()->WorldToScreen(entityWorldPos);
```

---

## Service Access

```cpp
// From IGameInstance
AssetManager& assets = GetAssetManager();
Input& input = GetInput();
AudioManager& audio = GetAudioManager();

// From Scene
IGameInstance& game = GetGameInstance();
SceneManager& scenes = GetSceneManager();
World* world = GetWorld();

// Quit application
GetGameInstance().RequestExit();
```

---

## Asset Loading

```cpp
// Synchronous (blocks until loaded)
SharedPtr<TextureResource> texture =
    AssetManager::GetInstance().LoadResource<TextureResource>("player.png");

// Asynchronous (planned)
AssetLoadRequest request;
request.assetPath = "level2_background.png";
request.type = EAssetType::Texture;
request.bLoadAsync = true;
request.onComplete = [](SharedPtr<IResource> resource) {
    // Called when loaded
    UMBRA_LOG_INFO("Background loaded!");
};

AssetStreamingManager::GetInstance().RequestAssetLoad(request);
```

---

## Debugging

```cpp
void MyScene::Update(float _deltaTime) {
    // Log entities
    World* world = GetWorld();
    Vector<EntityID> all = world->FindEntitiesByTag("Enemy");
    UMBRA_LOG_DEBUG("Enemy count: %zu", all.size());

    // Draw debug info
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.1f", 1.0f / _deltaTime);
    ImGui::Text("Delta: %.3f", _deltaTime);
    ImGui::Text("Enemies: %zu", all.size());
    ImGui::End();

    // Assertions
    UMBRA_ASSERT(mPlayer != MAX_ENTITY, "Player not spawned!");
}
```

---

## Common Component Snippets

### Transform

```cpp
world->AddComponent<TransformComponent>(entity,
    Math::Vector2f(x, y),        // position
    angleInRadians,              // rotation
    Math::Vector2f(sx, sy));     // scale
```

### Sprite

```cpp
world->AddComponent<SpriteComponent>(entity,
    "path/to/texture.png",       // texture path
    Color::White,                // tint color
    Math::Vector2f(32, 32));     // size
```

### Physics Body

```cpp
world->AddComponent<PhysicsBodyComponent>(entity,
    1.0f,                        // mass
    0.5f,                        // linear drag
    EBodyType::Dynamic);         // type (Dynamic/Static/Kinematic)
```

### Box Collider

```cpp
world->AddComponent<BoxColliderComponent>(entity,
    Math::Vector2f(32, 32));     // size
```

### Circle Collider

```cpp
world->AddComponent<CircleColliderComponent>(entity,
    16.0f);                      // radius
```

### Camera

```cpp
world->AddComponent<CameraComponent>(entity,
    20.0f,                       // orthographic size
    0.1f,                        // near clip
    100.0f);                     // far clip
```

---

## Lifecycle Method Reference

| Method | When Called | Purpose | Example Use |
|--------|-------------|---------|-------------|
| `Initialize()` | Once on scene creation | System setup | Register systems, configure world |
| `BeginPlay()` | When scene becomes active | Entity spawning | Create player, enemies, level geometry |
| `Update(dt)` | Every frame while running | Per-frame logic | Input handling, game logic |
| `FixedUpdate(dt)` | Fixed timestep while running | Physics logic | Apply forces, custom physics |
| `EndPlay()` | Before scene deactivation | Cleanup | Save checkpoint, log stats |
| `Shutdown()` | Before scene destruction | Resource cleanup | Release handles, clear caches |
| `Instantiate()` | When creating scene copy | Return new instance | `MakeUnique<MyScene>(*this)` |

---

## Scene State Reference

| State | Update | Render | When |
|-------|--------|--------|------|
| `Uninitialized` | ❌ | ❌ | Just created |
| `Initializing` | ❌ | ❌ | Initialize() running |
| `Ready` | ❌ | ❌ | Initialized, not started |
| `Running` | ✅ | ✅ | Active gameplay |
| `Paused` | ❌ | ✅ | Pause menu open |
| `Suspended` | ❌ | ❌ | In background (scene stack) |
| `Ending` | ❌ | ❌ | EndPlay() running |
| `Destroyed` | ❌ | ❌ | Shutdown, will be deleted |

---

## Best Practices

### Do's ✅

- **Always** implement `Instantiate()` to return a copy of your scene
- **Always** call base class methods if you override them
- **Use** SceneContext to pass data between scenes
- **Use** tags for entity queries
- **Create** camera in `BeginPlay()`, not `Initialize()`
- **Spawn** entities in `BeginPlay()`, not `Initialize()`
- **Register** systems in `Initialize()`, not `BeginPlay()`
- **Use** `FixedUpdate()` for physics-dependent logic
- **Clean up** in `EndPlay()` before scene transitions
- **Use** prefabs for repeated entity patterns

### Don'ts ❌

- **Don't** spawn entities in `Initialize()` (they won't persist across scene reloads)
- **Don't** store raw pointers to components (they may move in memory)
- **Don't** modify entities during iteration without deferring
- **Don't** call `Shutdown()` manually (SceneManager handles it)
- **Don't** use `std::move()` on SharedPtr (semantic error)
- **Don't** hardcode scene transitions (use scene IDs)
- **Don't** forget to implement `Instantiate()`
- **Don't** mix ownership models (stick to Unique or Shared, not both)

---

## Troubleshooting

### Scene Won't Load

```cpp
// Check if scene is registered
if (!GetSceneManager().HasSceneAsset("MyScene")) {
    UMBRA_LOG_ERROR("Scene not registered: MyScene");
}

// Make sure you registered in IGameInstance::Initialize()
void MyGame::Initialize() {
    GetSceneManager().RegisterSceneAsset("MyScene",
        MakeUnique<MyScene>());  // ← Don't forget this!
}
```

### Entities Not Visible

```cpp
// 1. Check camera is created
EntityID camera = GetMainCamera();
UMBRA_ASSERT(camera != MAX_ENTITY, "No camera!");

// 2. Check entity has Transform and Sprite
UMBRA_ASSERT(world->HasComponent<TransformComponent>(entity), "No transform!");
UMBRA_ASSERT(world->HasComponent<SpriteComponent>(entity), "No sprite!");

// 3. Check entity is in camera view
auto* transform = world->GetComponent<TransformComponent>(entity);
UMBRA_LOG_DEBUG("Entity position: (%.2f, %.2f)",
    transform->position.x, transform->position.y);
```

### OnEndPlay Not Called

This is a known bug in the current implementation. The new design ensures `EndPlay()` is always called before scene transitions.

---

For detailed design and implementation, see:
- [GameFrameworkDesign.md](GameFrameworkDesign.md) - Complete design
- [GameFrameworkSummary.md](GameFrameworkSummary.md) - Executive summary
- [GameFrameworkDiagrams.md](GameFrameworkDiagrams.md) - Visual diagrams
