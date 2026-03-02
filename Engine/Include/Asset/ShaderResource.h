#pragma once
#include "Asset/IResource.h"
#include "Graphics/IShader.h"
#include "Umbra.h"

namespace Umbra {

    // -------------------------------------------------------------------------
    // ShaderResource
    //
    // Stores a compiled SFML shader. The filePath key encodes which files to load:
    //   "path/to/frag.glsl"            — fragment shader only
    //   "path/to/vert.glsl|frag.glsl"  — vertex + fragment shader
    //
    // Use AssetManager::GetShader() / GetShaderAsync() to create instances.
    // -------------------------------------------------------------------------
    class ShaderResource : public IResource {
    public:
        ShaderResource(const String& _filePath);

        bool Load() override;
        void Unload() override;

        UniquePtr<IShader> mShader;
    };

    // -------------------------------------------------------------------------
    // Shader
    //
    // User-facing handle. Holds a shared reference to ShaderResource.
    // Uniform setters delegate through to the underlying IShader backend.
    // GetNativeHandle() returns sf::Shader* cast to void*.
    // -------------------------------------------------------------------------
    class Shader : public IResourceHandle<ShaderResource> {
    public:
        explicit Shader(SharedPtr<ShaderResource> _shaderResource);

        void* GetNativeHandle() const;

        void SetUniformFloat(const String& _name, float _val);
        void SetUniformInt(const String& _name, int _val);
        void SetUniformBool(const String& _name, bool _val);
        void SetUniformVec2(const String& _name, float _x, float _y);
        void SetUniformVec3(const String& _name, float _x, float _y, float _z);
        void SetUniformVec4(const String& _name, float _x, float _y, float _z, float _w);
    };

} // namespace Umbra
