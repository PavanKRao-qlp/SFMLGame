#include "Asset/FontResource.h"

#include "Graphics/Backends/SfmlFont.h"

namespace Umbra {

    FontResource::FontResource(const String& _filePath) : IResource(_filePath) {}

    bool FontResource::Load() {
        mFont = std::make_unique<SfmlFont>();
        return mFont->LoadFromFile(filePath);
    }

    void FontResource::Unload() {
        if (mFont) {
            mFont->Unload();
        }
    }

    Font::Font(SharedPtr<FontResource> _fontResource) : mRefFontResource(_fontResource) {}

    void* Font::GetNativeHandle() const {
        if (mRefFontResource && mRefFontResource->mFont) {
            return mRefFontResource->mFont->GetNativeHandle();
        }
        return nullptr;
    }

    bool Font::IsLoaded() const {
        return mRefFontResource && mRefFontResource->mFont != nullptr;
    }

} // namespace Umbra
