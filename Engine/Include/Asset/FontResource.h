#pragma once
#include "Asset/IResource.h"
#include "Graphics/IFont.h"
#include "Umbra.h"

namespace Umbra {

    class FontResource : public IResource {
    public:
        FontResource(const String& _filePath);

        bool Load() override;
        void Unload() override;

        UniquePtr<IFont> mFont;
    };

    class Font : public IResourceHandle<FontResource> {
    public:
        explicit Font(SharedPtr<FontResource> _fontResource);
        void* GetNativeHandle() const;
    };

} // namespace Umbra
