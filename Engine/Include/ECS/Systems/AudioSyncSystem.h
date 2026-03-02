#pragma once
#include "ECS/Components/AudioSource.h"
#include "ECS/ECSRegister.h"
#include "ECS/System.h"
#include "Service/Audio/AudioService.h"

namespace Umbra {

    /// @brief System that bridges ECS entities with the AudioService
    /// Handles sound creation, destruction, and volume synchronization
    class AudioSyncSystem : public System {
    public:
        explicit AudioSyncSystem(AudioService* _audioService);
        ~AudioSyncSystem();

        void Update() override;

        /// @brief Called when an entity with AudioSourceComponent is added
        void AddEntity(EntityID _entity) override;

        /// @brief Called when an entity with AudioSourceComponent is removed
        void RemoveEntity(EntityID _entity) override;

    private:
        /// @brief Creates a sound for an entity that needs one
        void CreateSoundForEntity(EntityID _entity);

        /// @brief Destroys a sound when entity is removed
        void DestroySoundForEntity(EntityID _entity);

    private:
        AudioService* mAudioService = nullptr;
    };

} // namespace Umbra
