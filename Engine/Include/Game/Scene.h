#pragma once
#include "Game/World.h"
namespace Umbra {
    class SceneAsset {};
    /**
     * Runtime Scene Class
     */
    class Scene {
    public:
        Scene();
        virtual ~Scene();
        void Construct();
        void Render();
        void Simulate();
        virtual void Initialize()                = 0;
        virtual void OnBeginPlay()               = 0;
        virtual void OnEndPlay()                 = 0;
        virtual void OnFixedUpdated()            = 0;
        virtual SharedPtr<Scene> InsatiateCopy() = 0;
        virtual void OnUpdate()                  = 0;
        virtual void ShutDown();
        bool IsLoaded();
        const String& GetSceneID();
        World* GetWorld();

        Scene(const Scene& _scene);

    protected:
        class IGameInstance& GetGameInstance();
        class SceneManager& GetSceneManager();
        EntityID GetCameraEntity();

    private:
        friend class SceneManager;
        void SetSceneId(String _sceneId);
        void SetGameInstance(class IGameInstance* _gameInstance);
        void SetSceneManager(class SceneManager* _sceneManager);
        EntityID mCameraEntity = MAX_ENTITY;
        String mSceneIdentifier;
        UniquePtr<World> mWorld;
        IGameInstance* mGameInstance;
        SceneManager* mSceneManager;
        bool bLoaded;
    };
} // namespace Umbra
