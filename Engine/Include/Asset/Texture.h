
#pragma once
#include "Asset/TextureResource.h"
#include "Graphics/ITexture.h"
#include "Math/Vector.h"
#include "Umbra.h"

namespace Umbra {
    class TextureResource : public IResource {
    public:
        TextureResource(const String& _filePath);

        bool Load() override;
        void Unload() override;

        UniquePtr<ITexture> mTexture;
    };

    class Texture {
    public:
        Texture(SharedPtr<TextureResource> _textureResource);
        void* GetNativeHandle();
        Math::Vector2i GetSize() const;

    private:
        SharedPtr<TextureResource> refTextureResource;
    };
} // namespace Umbra
