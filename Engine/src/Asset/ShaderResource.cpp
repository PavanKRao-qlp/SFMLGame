#include "Asset/ShaderResource.h"

#include "Graphics/Backends/SfmlShader.h"

namespace Umbra {

    // -------------------------------------------------------------------------
    // ShaderResource
    // -------------------------------------------------------------------------

    ShaderResource::ShaderResource(const String& _filePath) : IResource(_filePath) {}

    bool ShaderResource::Load() {
        mShader = std::make_unique<SfmlShader>();

        // A pipe separator encodes "vert|frag" in the combined cache key.
        auto sep = filePath.find('|');
        if (sep != String::npos) {
            String vertPath = filePath.substr(0, sep);
            String fragPath = filePath.substr(sep + 1);
            bLoaded = mShader->LoadVertFrag(vertPath, fragPath);
        } else {
            bLoaded = mShader->LoadFragment(filePath);
        }

        return bLoaded;
    }

    void ShaderResource::Unload() {
        if (mShader) {
            mShader->Unload();
        }
    }

    // -------------------------------------------------------------------------
    // Shader (user-facing handle)
    // -------------------------------------------------------------------------

    Shader::Shader(SharedPtr<ShaderResource> _shaderResource)
        : IResourceHandle<ShaderResource>(std::move(_shaderResource)) {}

    void* Shader::GetNativeHandle() const {
        if (mResource && mResource->mShader) {
            return mResource->mShader->GetNativeHandle();
        }
        return nullptr;
    }

    void Shader::SetUniformFloat(const String& _name, float _val) {
        if (mResource && mResource->mShader) {
            mResource->mShader->SetUniformFloat(_name, _val);
        }
    }

    void Shader::SetUniformInt(const String& _name, int _val) {
        if (mResource && mResource->mShader) {
            mResource->mShader->SetUniformInt(_name, _val);
        }
    }

    void Shader::SetUniformBool(const String& _name, bool _val) {
        if (mResource && mResource->mShader) {
            mResource->mShader->SetUniformBool(_name, _val);
        }
    }

    void Shader::SetUniformVec2(const String& _name, float _x, float _y) {
        if (mResource && mResource->mShader) {
            mResource->mShader->SetUniformVec2(_name, _x, _y);
        }
    }

    void Shader::SetUniformVec3(const String& _name, float _x, float _y, float _z) {
        if (mResource && mResource->mShader) {
            mResource->mShader->SetUniformVec3(_name, _x, _y, _z);
        }
    }

    void Shader::SetUniformVec4(const String& _name, float _x, float _y, float _z, float _w) {
        if (mResource && mResource->mShader) {
            mResource->mShader->SetUniformVec4(_name, _x, _y, _z, _w);
        }
    }

} // namespace Umbra
