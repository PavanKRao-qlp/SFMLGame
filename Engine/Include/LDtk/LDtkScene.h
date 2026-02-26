#pragma once
#include "Asset/Texture.h"
#include "EnginePCH.h"
#include "Game/Scene.h"
#include "Thread/Atomic.h"
#include "Thread/Mutex.h"

namespace Umbra {

    /// Internal data bag populated by the async load jobs and consumed on the main thread.
    /// Lives on the heap (SharedPtr) so it safely outlives the job lambdas.
    struct LDtkLoadData {
        struct TileEntry {
            float  x, y;           // centre position in world space
            float  width, height;  // tile pixel size
            String tilesetPath;    // resolved, normalised file path
            float  uvX, uvY, uvW, uvH; // normalised UV rect (0-1)
            int    zOrder;
        };

        struct EntityEntry {
            float                x, y;          // centre position in world space
            float                width, height;
            String               typeName;
            UMap<String, String> fields;         // LDtk custom fields serialised to strings
            int                  zOrder;
        };

        // Written by Job A (single worker); read-only afterwards
        Vector<TileEntry>   tiles;
        Vector<EntityEntry> entities;
        Vector<String>      uniqueTilesetPaths;

        // Written by Job B (parallel, mutex-protected); read by SpawnECSEntities()
        UMap<String, SharedPtr<Texture>> textures;
        Mutex                            texturesMutex;

        // Set by Job C (Release); polled by OnUpdate() on the main thread (Acquire)
        AtomicBool bReady{false};

        LDtkLoadData()                               = default;
        LDtkLoadData(const LDtkLoadData&)            = delete;
        LDtkLoadData& operator=(const LDtkLoadData&) = delete;
    };

    /// Concrete Scene that populates the ECS world from a LDtk level file.
    /// Users never subclass this — they call SceneManager::LoadLDTKScene().
    class LDtkScene : public Scene {
    public:
        /// @param _ldtkPath   Path to the .ldtk file (relative to the working directory).
        /// @param _levelName  Level name to load. Empty = load all levels.
        LDtkScene(const String& _ldtkPath, const String& _levelName = "");

        // Scene interface
        void             Initialize()   override;
        void             OnBeginPlay()  override {}
        void             OnEndPlay()    override {}
        void             OnFixedUpdate() override {}
        void             OnUpdate()     override;
        SharedPtr<Scene> InstantiateCopy() override;

    private:
        void SpawnECSEntities();

        String                  mLdtkPath;
        String                  mLevelName;    // empty = all levels
        SharedPtr<LDtkLoadData> mLoadData;
        bool                    mEntitiesSpawned = false;
    };

} // namespace Umbra
