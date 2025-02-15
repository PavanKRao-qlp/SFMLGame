#pragma once
#include "Game/World.h"
namespace Umbra {
    class SceneAsset {};
    class Scene {
    public:
        Scene();
        virtual ~Scene();
        void Construct();
        void Render();
        void Simulate();
        virtual void Initialize()     = 0;
        virtual void OnBeginPlay()    = 0;
        virtual void OnFixedUpdated() = 0;
        virtual void OnUpdate()       = 0;
        // virtual void OnEndPlay()      = 0;
        virtual void ShutDown();
        bool IsLoaded();
        const String& GetSceneID();
        World* GetWorld();

        void SetGameInstance(class IGameInstance* _gameInstance);
        class IGameInstance* GetGameInstance();
        EntityID GetCameraEntity();


    private:
        EntityID mCameraEntity = MAX_ENTITY;
        String mSceneIdentifier;
        UniquePtr<World> mWorld;
        SharedPtr<class IGameInstance> mGameInstance;
        bool bLoaded;
    };
} // namespace Umbra
