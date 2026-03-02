# Game Framework - Visual Diagrams

## Architecture Overview

### Component Hierarchy

```
┌─────────────────────────────────────────────────────────────┐
│                      Application Layer                       │
│                                                               │
│  ┌────────────────────────────────────────────────────────┐ │
│  │                    IGameInstance                       │ │
│  │                                                         │ │
│  │  - Configuration (FGameConfig)                         │ │
│  │  - Global state & services                             │ │
│  │  - Lifecycle: Initialize() → Update() → Shutdown()    │ │
│  │                                                         │ │
│  │  Owns: UniquePtr<SceneManager>                        │ │
│  └──────────────────┬─────────────────────────────────────┘ │
│                     │                                        │
└─────────────────────┼────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│                    Scene Management Layer                    │
│                                                               │
│  ┌────────────────────────────────────────────────────────┐ │
│  │                    SceneManager                        │ │
│  │                                                         │ │
│  │  Scene Templates (Master Copies):                      │ │
│  │  ┌──────────────────────────────────────────────────┐ │ │
│  │  │ UMap<String, UniquePtr<Scene>>                   │ │ │
│  │  │  "MainMenu"  → MainMenuScene template            │ │ │
│  │  │  "Level1"    → Level1Scene template              │ │ │
│  │  │  "PauseMenu" → PauseMenuScene template           │ │ │
│  │  └──────────────────────────────────────────────────┘ │ │
│  │                                                         │ │
│  │  Active Scene Stack (Instances):                       │ │
│  │  ┌──────────────────────────────────────────────────┐ │ │
│  │  │ Vector<UniquePtr<Scene>>                         │ │ │
│  │  │  [0] Level1Scene (Running)                       │ │ │
│  │  │  [1] PauseMenuScene (Running) ← Active           │ │ │
│  │  └──────────────────────────────────────────────────┘ │ │
│  │                                                         │ │
│  │  Preloaded Scenes:                                      │ │
│  │  ┌──────────────────────────────────────────────────┐ │ │
│  │  │ UMap<String, UniquePtr<Scene>>                   │ │ │
│  │  │  "Level2" → Preloaded Level2Scene instance       │ │ │
│  │  └──────────────────────────────────────────────────┘ │ │
│  │                                                         │ │
│  │  Methods:                                               │ │
│  │  - RegisterSceneAsset(id, template)                    │ │
│  │  - LoadScene(id, options)                              │ │
│  │  - LoadSceneAdditive(id, options)                      │ │
│  │  - PreloadScene(id)                                     │ │
│  │  - PauseScene(scene) / ResumeScene(scene)              │ │
│  └──────────────────┬─────────────────────────────────────┘ │
│                     │                                        │
└─────────────────────┼────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│                      Scene Runtime Layer                     │
│                                                               │
│  ┌────────────────────────────────────────────────────────┐ │
│  │                        Scene                           │ │
│  │                                                         │ │
│  │  State: ESceneState (Uninitialized → Running)         │ │
│  │  ID: String ("Level1")                                 │ │
│  │  Context: SceneContext (data from previous scene)      │ │
│  │  Camera: EntityID                                      │ │
│  │                                                         │ │
│  │  Lifecycle:                                             │ │
│  │  1. Initialize()      → System registration           │ │
│  │  2. BeginPlay()       → Entity spawning               │ │
│  │  3. Update(dt)        → Per-frame logic               │ │
│  │  4. FixedUpdate(dt)   → Physics logic                 │ │
│  │  5. EndPlay()         → Cleanup                       │ │
│  │  6. Shutdown()        → Resource release              │ │
│  │                                                         │ │
│  │  Owns: UniquePtr<World>                               │ │
│  └──────────────────┬─────────────────────────────────────┘ │
│                     │                                        │
└─────────────────────┼────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│                        ECS Layer                             │
│                                                               │
│  ┌────────────────────────────────────────────────────────┐ │
│  │                        World                           │ │
│  │                                                         │ │
│  │  Configuration: WorldConfig                            │ │
│  │  - bCreateRenderSystem                                 │ │
│  │  - bCreatePhysicsSystem                                │ │
│  │  - bCreateCameraSystem                                 │ │
│  │  - maxEntities                                         │ │
│  │                                                         │ │
│  │  Owns: SharedPtr<ECSRegister>                         │ │
│  │  ┌──────────────────────────────────────────────────┐ │ │
│  │  │            ECSRegister                           │ │ │
│  │  │                                                   │ │ │
│  │  │  EntityManager:                                   │ │ │
│  │  │    Entities: [0, 1, 2, 3, ..., 9999]            │ │ │
│  │  │                                                   │ │ │
│  │  │  ComponentManager:                                │ │ │
│  │  │    ComponentArray<Transform>                     │ │ │
│  │  │    ComponentArray<Sprite>                        │ │ │
│  │  │    ComponentArray<PhysicsBody>                   │ │ │
│  │  │    ComponentArray<BoxCollider>                   │ │ │
│  │  │    ...                                            │ │ │
│  │  │                                                   │ │ │
│  │  │  SystemManager:                                   │ │ │
│  │  │    FrameStart:    [InputSystem]                  │ │ │
│  │  │    Simulation:    [PhysicsSystem, AISystem]      │ │ │
│  │  │    PreRender:     [CameraSystem]                 │ │ │
│  │  │    Render:        [RenderSystem]                 │ │ │
│  │  │    FrameEnd:      [LifeTimeSystem]               │ │ │
│  │  └──────────────────────────────────────────────────┘ │ │
│  │                                                         │ │
│  │  Prefab System:                                         │ │
│  │  ┌──────────────────────────────────────────────────┐ │ │
│  │  │ UMap<String, EntityTemplate>                     │ │ │
│  │  │  "BasicEnemy"  → Template with components        │ │ │
│  │  │  "Player"      → Template with components        │ │ │
│  │  └──────────────────────────────────────────────────┘ │ │
│  │                                                         │ │
│  │  Layer System:                                          │ │
│  │  ┌──────────────────────────────────────────────────┐ │ │
│  │  │ LayerManager                                     │ │ │
│  │  │  "Background"  (priority: 0)   → [Entity 1, 2]  │ │ │
│  │  │  "Gameplay"    (priority: 10)  → [Entity 3, 4]  │ │ │
│  │  │  "UI"          (priority: 100) → [Entity 5, 6]  │ │ │
│  │  └──────────────────────────────────────────────────┘ │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                               │
└───────────────────────────────────────────────────────────────┘
```

---

## Scene Lifecycle State Machine

```
┌─────────────────┐
│ Uninitialized   │  (Scene just created)
└────────┬────────┘
         │ SceneManager::InstantiateScene()
         ▼
┌─────────────────┐
│ Initializing    │  (Initialize() running)
└────────┬────────┘
         │ Initialize() completes
         ▼
┌─────────────────┐
│     Ready       │  (Initialized but not active)
└────────┬────────┘
         │ SceneManager activates scene
         ▼
┌─────────────────┐
│    Running      │◄──┐ (BeginPlay called, actively updating)
└────────┬────────┘   │
         │             │ ResumeScene()
         │ PauseScene()│
         ▼             │
┌─────────────────┐   │
│     Paused      │───┘ (Update stopped, still rendered)
└────────┬────────┘
         │ SuspendScene()
         ▼
┌─────────────────┐
│   Suspended     │ (Update and render stopped)
└────────┬────────┘
         │ SceneManager::UnloadScene()
         ▼
┌─────────────────┐
│     Ending      │ (EndPlay() running)
└────────┬────────┘
         │ EndPlay() completes
         ▼
┌─────────────────┐
│    Destroyed    │ (Shutdown() called, scene deleted)
└─────────────────┘
```

---

## Scene Transition Flow

### Simple Scene Transition

```
Current State:           Transition:              New State:
┌──────────────┐        ┌──────────────┐        ┌──────────────┐
│   Level1     │        │  Transition  │        │   Level2     │
│  (Running)   │   →    │   (Fade)     │   →    │  (Running)   │
└──────────────┘        └──────────────┘        └──────────────┘

Timeline:
t=0.0s  EndPlay() on Level1
t=0.0s  FadeOut starts (0.5s)
t=0.5s  Shutdown() on Level1, deallocate
t=0.5s  Instantiate Level2 from template
t=0.5s  Initialize() on Level2
t=0.5s  FadeIn starts (0.5s)
t=0.5s  BeginPlay() on Level2
t=1.0s  Transition complete, Level2 running
```

### Additive Scene Loading (Overlay)

```
Current State:           Add Overlay:             Stacked State:
┌──────────────┐        ┌──────────────┐        ┌──────────────┐
│   Level1     │        │  PauseMenu   │        │  PauseMenu   │ ← Active
│  (Running)   │   +    │   (Load)     │   =    │  (Running)   │
└──────────────┘        └──────────────┘        ├──────────────┤
                                                 │   Level1     │
                                                 │  (Paused)    │
                                                 └──────────────┘

Scene Stack:
Index 0: Level1  (Paused, still rendered)
Index 1: PauseMenu (Active, receives input)

When PauseMenu closes:
- UnloadScene("PauseMenu")
- ResumeScene(Level1)
- Level1 becomes active again
```

---

## Data Flow: Scene Context Passing

```
┌─────────────────────────────────────────────────────────────┐
│                       Level1Scene                            │
│                                                               │
│  Player defeats boss:                                         │
│  - playerHealth = 85                                          │
│  - levelScore = 1500                                          │
│  - unlockedWeapon = "LaserGun"                                │
│                                                               │
│  Transition to Level2:                                        │
│  ┌─────────────────────────────────────────────────────┐    │
│  │ SceneContext context;                                │    │
│  │ context.SetInt("playerHealth", 85);                  │    │
│  │ context.SetInt("levelScore", 1500);                  │    │
│  │ context.SetInt("levelNumber", 2);                    │    │
│  │ context.SetString("unlockedWeapon", "LaserGun");     │    │
│  │                                                       │    │
│  │ SceneLoadOptions options;                            │    │
│  │ options.context = context;                           │    │
│  │ options.transition = ESceneTransition::FadeInOut;    │    │
│  │                                                       │    │
│  │ GetSceneManager().LoadScene("Level2", options);      │    │
│  └─────────────────────────────────────────────────────┘    │
└───────────────────────────────┬─────────────────────────────┘
                                │
                                │ SceneContext passed
                                │
                                ▼
┌─────────────────────────────────────────────────────────────┐
│                       Level2Scene                            │
│                                                               │
│  BeginPlay():                                                 │
│  ┌─────────────────────────────────────────────────────┐    │
│  │ const SceneContext& ctx = GetSceneContext();        │    │
│  │                                                       │    │
│  │ int health = ctx.GetInt("playerHealth", 100);       │    │
│  │ int score = ctx.GetInt("levelScore", 0);            │    │
│  │ int level = ctx.GetInt("levelNumber", 1);           │    │
│  │ String weapon = ctx.GetString("unlockedWeapon");    │    │
│  │                                                       │    │
│  │ SpawnPlayer(health);                                 │    │
│  │ UpdateScoreDisplay(score);                           │    │
│  │ EquipWeapon(weapon);                                 │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

---

## Prefab System Flow

```
┌───────────────────────────────────────────────────────────┐
│                    Prefab Registration                     │
│                                                             │
│  1. Define EntityTemplate                                  │
│  ┌─────────────────────────────────────────────────────┐  │
│  │ EntityTemplate enemyTemplate;                       │  │
│  │ enemyTemplate.name = "BasicEnemy";                  │  │
│  │ enemyTemplate.components = {                        │  │
│  │    TransformComponent{...},                         │  │
│  │    SpriteComponent{...},                            │  │
│  │    PhysicsBodyComponent{...},                       │  │
│  │    BoxColliderComponent{...},                       │  │
│  │    EnemyAIComponent{...}                            │  │
│  │ };                                                   │  │
│  │ enemyTemplate.tags = {"Enemy", "Hostile"};          │  │
│  └─────────────────────────────────────────────────────┘  │
│                                                             │
│  2. Register with World                                    │
│  ┌─────────────────────────────────────────────────────┐  │
│  │ world->RegisterPrefab("BasicEnemy", enemyTemplate); │  │
│  └─────────────────────────────────────────────────────┘  │
└───────────────────────────────┬─────────────────────────────┘
                                │
                                │ Template stored in World
                                │
                                ▼
┌───────────────────────────────────────────────────────────┐
│                    Prefab Instantiation                    │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐  │
│  │ // Spawn 5 enemies                                  │  │
│  │ for (int i = 0; i < 5; ++i) {                       │  │
│  │     EntityID enemy =                                │  │
│  │         world->InstantiatePrefab("BasicEnemy");     │  │
│  │                                                      │  │
│  │     // Customize instance                           │  │
│  │     auto* transform =                               │  │
│  │         world->GetComponent<Transform>(enemy);      │  │
│  │     transform->position = spawnPositions[i];        │  │
│  │ }                                                    │  │
│  └─────────────────────────────────────────────────────┘  │
│                                                             │
│  Result:                                                    │
│  ┌──────────┬──────────┬──────────┬──────────┬──────────┐ │
│  │ Enemy 0  │ Enemy 1  │ Enemy 2  │ Enemy 3  │ Enemy 4  │ │
│  │ @(0,0)   │ @(5,0)   │ @(10,0)  │ @(15,0)  │ @(20,0)  │ │
│  └──────────┴──────────┴──────────┴──────────┴──────────┘ │
└───────────────────────────────────────────────────────────┘
```

---

## Layer System Execution

```
┌───────────────────────────────────────────────────────────┐
│                      Scene Layers                          │
│                                                             │
│  Layer Setup:                                              │
│  ┌─────────────────────────────────────────────────────┐  │
│  │ LayerManager& layers = world->GetLayerManager();    │  │
│  │ layers.CreateLayer("Background", 0);                │  │
│  │ layers.CreateLayer("Gameplay", 10);                 │  │
│  │ layers.CreateLayer("Effects", 50);                  │  │
│  │ layers.CreateLayer("UI", 100);                      │  │
│  └─────────────────────────────────────────────────────┘  │
│                                                             │
│  Entity Assignment:                                         │
│  ┌─────────────────────────────────────────────────────┐  │
│  │ layers.AssignEntityToLayer(background, "Background");│  │
│  │ layers.AssignEntityToLayer(player, "Gameplay");     │  │
│  │ layers.AssignEntityToLayer(particle, "Effects");    │  │
│  │ layers.AssignEntityToLayer(hud, "UI");              │  │
│  └─────────────────────────────────────────────────────┘  │
└───────────────────────────────────────────────────────────┘
                                │
                                ▼
┌───────────────────────────────────────────────────────────┐
│                     Render Order                           │
│                                                             │
│  Priority 0:    ┌─────────────────┐                       │
│                 │   Background     │  (mountains, sky)     │
│                 └─────────────────┘                       │
│                                                             │
│  Priority 10:   ┌─────────────────┐                       │
│                 │    Gameplay      │  (player, enemies)    │
│                 └─────────────────┘                       │
│                                                             │
│  Priority 50:   ┌─────────────────┐                       │
│                 │    Effects       │  (particles, VFX)     │
│                 └─────────────────┘                       │
│                                                             │
│  Priority 100:  ┌─────────────────┐                       │
│                 │       UI         │  (HUD, menus)         │
│                 └─────────────────┘                       │
│                                                             │
│  Result: Background → Gameplay → Effects → UI              │
└───────────────────────────────────────────────────────────┘
```

---

## Save/Load System Flow

```
┌───────────────────────────────────────────────────────────┐
│                       Save Game                            │
│                                                             │
│  Current Game State:                                       │
│  - Scene: "Level3"                                         │
│  - Player Health: 65                                       │
│  - Player Score: 5000                                      │
│  - Level Progress: 3                                       │
│  - Inventory: ["Key", "Sword", "Potion"]                   │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐  │
│  │ SaveData saveData;                                  │  │
│  │ saveData.sceneName = "Level3";                      │  │
│  │ saveData.sceneContext.SetInt("playerHealth", 65);  │  │
│  │ saveData.sceneContext.SetInt("playerScore", 5000); │  │
│  │ saveData.sceneContext.SetInt("levelProgress", 3);  │  │
│  │ // ... serialize entities and inventory ...        │  │
│  │                                                      │  │
│  │ SaveSystem saveSystem;                              │  │
│  │ saveSystem.SaveGame("Slot1", saveData);            │  │
│  └─────────────────────────────────────────────────────┘  │
│                                                             │
│  File Written: saves/Slot1.sav                             │
└───────────────────────────────┬───────────────────────────┘
                                │
                                │ Game closes, later reopened
                                │
                                ▼
┌───────────────────────────────────────────────────────────┐
│                       Load Game                            │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐  │
│  │ SaveSystem saveSystem;                              │  │
│  │ SaveData loadedData;                                │  │
│  │                                                      │  │
│  │ if (saveSystem.LoadGame("Slot1", loadedData)) {    │  │
│  │     SceneLoadOptions options;                       │  │
│  │     options.context = loadedData.sceneContext;     │  │
│  │     options.transition = ESceneTransition::FadeIn; │  │
│  │                                                      │  │
│  │     GetSceneManager().LoadScene(                    │  │
│  │         loadedData.sceneName, options);            │  │
│  │ }                                                    │  │
│  └─────────────────────────────────────────────────────┘  │
│                                                             │
│  Result: Player spawns in Level3 with saved state          │
│  - Health: 65                                              │
│  - Score: 5000                                             │
│  - Progress: 3                                             │
│  - Inventory restored                                       │
└───────────────────────────────────────────────────────────┘
```

---

## Complete Game Flow Example

```
┌─────────────────────────────────────────────────────────────┐
│ 1. Application Startup                                       │
│    ┌──────────────────────────────────────────────────┐    │
│    │ CreateApplication() → MyGameInstance              │    │
│    │ App::Initialize()                                 │    │
│    │   → IGameInstance::Initialize()                   │    │
│    └──────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                           ▼
┌─────────────────────────────────────────────────────────────┐
│ 2. Scene Registration                                        │
│    ┌──────────────────────────────────────────────────┐    │
│    │ GetSceneManager().RegisterSceneAsset(             │    │
│    │     "MainMenu", MakeUnique<MainMenuScene>());     │    │
│    │ GetSceneManager().RegisterSceneAsset(             │    │
│    │     "Level1", MakeUnique<Level1Scene>());         │    │
│    │ GetSceneManager().RegisterSceneAsset(             │    │
│    │     "PauseMenu", MakeUnique<PauseMenuScene>());   │    │
│    └──────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                           ▼
┌─────────────────────────────────────────────────────────────┐
│ 3. Load Initial Scene                                        │
│    ┌──────────────────────────────────────────────────┐    │
│    │ GetSceneManager().LoadScene("MainMenu");          │    │
│    │   → Instantiate MainMenuScene from template       │    │
│    │   → Initialize()                                  │    │
│    │   → BeginPlay()                                   │    │
│    └──────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                           ▼
┌─────────────────────────────────────────────────────────────┐
│ 4. Main Menu Running                                         │
│    ┌──────────────────────────────────────────────────┐    │
│    │ Loop: Update(dt) every frame                      │    │
│    │ User clicks "Play" button                         │    │
│    └──────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                           ▼
┌─────────────────────────────────────────────────────────────┐
│ 5. Transition to Level1                                      │
│    ┌──────────────────────────────────────────────────┐    │
│    │ SceneLoadOptions options;                         │    │
│    │ options.transition = ESceneTransition::FadeInOut; │    │
│    │ GetSceneManager().LoadScene("Level1", options);   │    │
│    │                                                    │    │
│    │ MainMenu: EndPlay() → Shutdown()                  │    │
│    │ Level1: Instantiate → Initialize() → BeginPlay()  │    │
│    └──────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                           ▼
┌─────────────────────────────────────────────────────────────┐
│ 6. Level1 Running                                            │
│    ┌──────────────────────────────────────────────────┐    │
│    │ Loop: Update(dt) + FixedUpdate(fixedDt)           │    │
│    │ User presses ESC                                  │    │
│    └──────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                           ▼
┌─────────────────────────────────────────────────────────────┐
│ 7. Open Pause Menu (Additive)                               │
│    ┌──────────────────────────────────────────────────┐    │
│    │ SceneLoadOptions pauseOpts;                       │    │
│    │ pauseOpts.bUnloadCurrent = false;                │    │
│    │ pauseOpts.bPauseCurrent = true;                  │    │
│    │ GetSceneManager().LoadSceneAdditive(              │    │
│    │     "PauseMenu", pauseOpts);                      │    │
│    │                                                    │    │
│    │ Scene Stack:                                       │    │
│    │   [0] Level1 (Paused)                             │    │
│    │   [1] PauseMenu (Active)                          │    │
│    └──────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                           ▼
┌─────────────────────────────────────────────────────────────┐
│ 8. Resume Gameplay                                           │
│    ┌──────────────────────────────────────────────────┐    │
│    │ User clicks "Resume" button                       │    │
│    │ GetSceneManager().UnloadScene("PauseMenu");       │    │
│    │ GetSceneManager().ResumeScene(level1Scene);       │    │
│    │                                                    │    │
│    │ Scene Stack:                                       │    │
│    │   [0] Level1 (Running)                            │    │
│    └──────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                           ▼
┌─────────────────────────────────────────────────────────────┐
│ 9. Level Complete                                            │
│    ┌──────────────────────────────────────────────────┐    │
│    │ All enemies defeated                              │    │
│    │ SceneContext ctx;                                 │    │
│    │ ctx.SetInt("playerHealth", currentHealth);        │    │
│    │ ctx.SetInt("levelNumber", 2);                     │    │
│    │                                                    │    │
│    │ GetSceneManager().LoadScene("Level2", {ctx});     │    │
│    └──────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                           ▼
┌─────────────────────────────────────────────────────────────┐
│ 10. Application Shutdown                                     │
│    ┌──────────────────────────────────────────────────┐    │
│    │ User closes window or quits                       │    │
│    │ CurrentScene: EndPlay() → Shutdown()              │    │
│    │ IGameInstance::Shutdown()                         │    │
│    │ App cleanup and exit                              │    │
│    └──────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

---

## Thread Safety Considerations

```
┌─────────────────────────────────────────────────────────────┐
│                      Main Thread                             │
│                                                               │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ Game Loop (60 FPS)                                     │ │
│  │                                                         │ │
│  │ 1. Input::Update()                                     │ │
│  │ 2. SceneManager::Update(dt)                            │ │
│  │    └─> Scene::Update(dt)                               │ │
│  │         └─> World::Update()                            │ │
│  │              └─> Systems execute                       │ │
│  │ 3. SceneManager::Render()                              │ │
│  │    └─> Scene::Render()                                 │ │
│  │         └─> World::Render()                            │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                                │
                                │ Async requests
                                ▼
┌─────────────────────────────────────────────────────────────┐
│                    Background Threads                        │
│                                                               │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ Asset Streaming Thread                                 │ │
│  │ - Load textures asynchronously                         │ │
│  │ - Load audio files                                     │ │
│  │ - Decompress resources                                 │ │
│  │                                                         │ │
│  │ Thread-safe queue:                                     │ │
│  │   AssetLoadRequest → Load → Complete → Notify main    │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                               │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ Scene Preload Thread                                   │ │
│  │ - Instantiate scene                                    │ │
│  │ - Initialize() (system setup)                          │ │
│  │ - Load required assets                                 │ │
│  │                                                         │ │
│  │ Note: BeginPlay() deferred to main thread             │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘

Synchronization Points:
- Asset completion callbacks execute on main thread
- Preloaded scenes activated only on main thread
- ECS operations are NOT thread-safe (main thread only)
```

---

This visual reference should help understand the architecture and data flows in the new Game Framework design.
