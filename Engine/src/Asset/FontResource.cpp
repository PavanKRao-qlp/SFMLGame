#include "Asset/FontResource.h"

#include "Graphics/Backends/SfmlFont.h"

namespace Umbra {

    FontResource::FontResource(const String& _filePath) : IResource(_filePath) {}

    bool FontResource::Load() {
        mFont   = std::make_unique<SfmlFont>();
        bLoaded = mFont->LoadFromFile(filePath);
        return bLoaded;
    }

    void FontResource::Unload() {
        if (mFont) {
            mFont->Unload();
        }
    }

    Font::Font(SharedPtr<FontResource> _fontResource)
        : IResourceHandle<FontResource>(std::move(_fontResource)) {}

    void* Font::GetNativeHandle() const {
        if (mResource && mResource->mFont) {
            return mResource->mFont->GetNativeHandle();
        }
        return nullptr;
    }

} // namespace Umbra
