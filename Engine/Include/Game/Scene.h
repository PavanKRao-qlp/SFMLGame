#pragma once
#include "Game/World.h"
namespace Umbra {
    class SceneAsset {};
    class Scene {
    public:
        Scene();
        virtual ~Scene();
        void Construct();
        virtual void Initialize() = 0;

        virtual void OnBeginPlay();
        virtual void OnFixedUpdated();
        virtual void OnUpdate();
        virtual void OnEndPlay();
        virtual void ShutDown();
        bool IsLoaded();
        const String& GetSceneID();
        const World& GetWorld();
        class IGameInstance* GetGameInstance();

    private:
        String mSceneIdentifier;
        UniquePtr<World> mWorld;
        SharedPtr<class IGameInstance> mGameInstance;
        bool bLoaded;
    };
} // namespace Umbra
