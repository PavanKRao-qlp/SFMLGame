#include "Graphics/Backends/SfmlShader.h"

#include <SFML/Graphics/Glsl.hpp>

namespace Umbra {

    bool SfmlShader::LoadFragment(const String& _fragPath) {
        mShader = std::make_unique<sf::Shader>();
        return mShader->loadFromFile(_fragPath, sf::Shader::Fragment);
    }

    bool SfmlShader::LoadVertFrag(const String& _vertPath, const String& _fragPath) {
        mShader = std::make_unique<sf::Shader>();
        return mShader->loadFromFile(_vertPath, _fragPath);
    }

    void SfmlShader::Unload() {
        mShader.reset();
    }

    void* SfmlShader::GetNativeHandle() const {
        return mShader.get();
    }

    void SfmlShader::SetUniformFloat(const String& _name, float _val) {
        if (mShader) mShader->setUniform(_name, _val);
    }

    void SfmlShader::SetUniformInt(const String& _name, int _val) {
        if (mShader) mShader->setUniform(_name, _val);
    }

    void SfmlShader::SetUniformBool(const String& _name, bool _val) {
        if (mShader) mShader->setUniform(_name, _val);
    }

    void SfmlShader::SetUniformVec2(const String& _name, float _x, float _y) {
        if (mShader) mShader->setUniform(_name, sf::Glsl::Vec2(_x, _y));
    }

    void SfmlShader::SetUniformVec3(const String& _name, float _x, float _y, float _z) {
        if (mShader) mShader->setUniform(_name, sf::Glsl::Vec3(_x, _y, _z));
    }

    void SfmlShader::SetUniformVec4(const String& _name, float _x, float _y, float _z, float _w) {
        if (mShader) mShader->setUniform(_name, sf::Glsl::Vec4(_x, _y, _z, _w));
    }

} // namespace Umbra
