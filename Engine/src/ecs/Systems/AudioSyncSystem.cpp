#include "ECS/Systems/AudioSyncSystem.h"

#include "Umbra.h"

namespace Umbra {

    AudioSyncSystem::AudioSyncSystem(AudioService* _audioService)
        : System(std::make_unique<ECView<AudioSourceComponent>>()), mAudioService(_audioService) {}

    AudioSyncSystem::~AudioSyncSystem() {
        // Destroy all sounds owned by entities in our view
        for (EntityID entity : mView->mEntities) {
            DestroySoundForEntity(entity);
        }
    }

    void AudioSyncSystem::Update() {
        if (mAudioService == nullptr) {
            return;
        }

        // Phase 1: Create sounds for entities that need them
        for (EntityID entity : mView->mEntities) {
            AudioSourceComponent* audio = mView->ecsRegister->GetComponent<AudioSourceComponent>(entity);
            if (audio->bNeedsSoundCreation && audio->bAutoPlay) {
                CreateSoundForEntity(entity);
            }
        }

        // Phase 2: Sync volume changes for active sounds
        for (EntityID entity : mView->mEntities) {
            AudioSourceComponent* audio = mView->ecsRegister->GetComponent<AudioSourceComponent>(entity);
            if (!audio->HasValidSound() || !audio->bSyncEnabled) {
                continue;
            }

            // Sync per-source volume if it changed
            mAudioService->SetSoundVolume(audio->Handle, audio->Volume);
        }

        // Phase 3: Clean up finished one-shot sounds
        mAudioService->Update();
    }

    void AudioSyncSystem::AddEntity(EntityID _entity) {
        System::AddEntity(_entity);

        // Mark entity as needing sound creation
        AudioSourceComponent* audio = mView->ecsRegister->GetComponent<AudioSourceComponent>(_entity);
        if (audio) {
            audio->bNeedsSoundCreation = true;
        }
    }

    void AudioSyncSystem::RemoveEntity(EntityID _entity) {
        DestroySoundForEntity(_entity);
        System::RemoveEntity(_entity);
    }

    void AudioSyncSystem::CreateSoundForEntity(EntityID _entity) {
        if (mAudioService == nullptr) {
            return;
        }

        AudioSourceComponent* audio = mView->ecsRegister->GetComponent<AudioSourceComponent>(_entity);
        if (audio == nullptr) {
            return;
        }

        // If already has a valid sound, skip
        if (audio->Handle.IsValid() && mAudioService->IsSoundValid(audio->Handle)) {
            audio->bNeedsSoundCreation = false;
            return;
        }

        // Don't create if no file path set
        if (audio->FilePath.empty()) {
            return;
        }

        // Create and play sound via service
        audio->Handle             = mAudioService->PlaySound(audio->FilePath, audio->Group, audio->bLooping);
        audio->bNeedsSoundCreation = false;

        // Apply per-source volume
        if (audio->Handle.IsValid()) {
            mAudioService->SetSoundVolume(audio->Handle, audio->Volume);
        }
    }

    void AudioSyncSystem::DestroySoundForEntity(EntityID _entity) {
        if (mAudioService == nullptr) {
            return;
        }

        AudioSourceComponent* audio = mView->ecsRegister->GetComponent<AudioSourceComponent>(_entity);
        if (audio == nullptr) {
            return;
        }

        if (audio->Handle.IsValid()) {
            mAudioService->StopSound(audio->Handle);
            audio->Handle = SoundHandle::Invalid();
        }
    }

} // namespace Umbra
