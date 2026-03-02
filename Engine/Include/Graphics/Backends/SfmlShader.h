#pragma once
#include "Graphics/IShader.h"

#include <SFML/Graphics/Shader.hpp>

namespace Umbra {

    class SfmlShader : public IShader {
    public:
        SfmlShader()  = default;
        ~SfmlShader() = default;

        bool LoadFragment(const String& _fragPath) override;
        bool LoadVertFrag(const String& _vertPath, const String& _fragPath) override;
        void Unload() override;
        void* GetNativeHandle() const override;

        void SetUniformFloat(const String& _name, float _val) override;
        void SetUniformInt(const String& _name, int _val) override;
        void SetUniformBool(const String& _name, bool _val) override;
        void SetUniformVec2(const String& _name, float _x, float _y) override;
        void SetUniformVec3(const String& _name, float _x, float _y, float _z) override;
        void SetUniformVec4(const String& _name, float _x, float _y, float _z, float _w) override;

        sf::Shader* GetSfmlShader() { return mShader.get(); }

    private:
        UniquePtr<sf::Shader> mShader;
    };

} // namespace Umbra
