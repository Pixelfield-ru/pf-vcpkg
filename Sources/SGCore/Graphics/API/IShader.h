//
// Created by stuka on 13.05.2023.
//
#ifndef NATIVECORE_ISHADER_H
#define NATIVECORE_ISHADER_H

#include <list>
#include <glm/matrix.hpp>

#include "SGCore/Memory/Assets/FileAsset.h"
#include "SGCore/Memory/Assets/IAssetObserver.h"

#include "IUniformBuffer.h"
#include "ITexture2D.h"
#include "ShaderDefine.h"

#include "SGCore/Memory/Assets/Materials/IMaterial.h"

#include "ShaderMarkup.h"

namespace SGCore
{
    class MaterialTexture;

    class IUniformBuffer;
    class IFrameBuffer;

    // todo: add various types of defines like material textures block define e.t.c.
    class IShader : public IAssetObserver
    {
    public:
        std::string m_version;

        Scope<IUniformBuffer> m_uniformBuffer;

        Weak<FileAsset> m_fileAsset;

        bool m_useMaterialSettings = true;
        bool m_bindFrameBuffers = true;

        virtual ~IShader() = default;

        virtual void destroy() = 0;

        virtual void bind() = 0;

        virtual void compile(Ref<FileAsset> asset) = 0;

        [[nodiscard]] virtual std::int32_t getShaderUniformLocation(const std::string& uniformName) = 0;

        // TODO: CLEAR THIS
        void addDefines(const SGShaderDefineType& shaderDefineType, const std::vector<ShaderDefine>& shaderDefines);
        void emplaceDefines(const SGShaderDefineType& shaderDefineType, std::vector<ShaderDefine>& shaderDefines);

        void addDefine(const SGShaderDefineType& shaderDefineType, const ShaderDefine& shaderDefine);
        void emplaceDefine(const SGShaderDefineType& shaderDefineType, ShaderDefine&& shaderDefine);

        void removeDefine(const SGShaderDefineType& shaderDefineType, const ShaderDefine& shaderDefine);
        void removeDefine(const SGShaderDefineType& shaderDefineType, const std::string& shaderDefineName);

        void updateDefine(const SGShaderDefineType& shaderDefineType, const ShaderDefine& shaderDefine);
        void emplaceUpdateDefine(const SGShaderDefineType& shaderDefineType, ShaderDefine&& shaderDefine);

        void updateDefines(const SGShaderDefineType& shaderDefineType, const std::vector<ShaderDefine>& shaderDefines);
        void emplaceUpdateDefines(const SGShaderDefineType& shaderDefineType, std::vector<ShaderDefine>& shaderDefines);

        void replaceDefines(const SGShaderDefineType& shaderDefineType, const std::list<ShaderDefine>& otherDefines) noexcept;
        void replaceDefines(const SGShaderDefineType& shaderDefineType, Ref<IShader> otherShader) noexcept;

        void clearDefinesOfType(const SGShaderDefineType& shaderDefineType) noexcept;

        /**
         * Calls recompile of shader program.
         */
        void onAssetModified() override;

        /**
         * Calls recompile of shader program.
         */
        void onAssetPathChanged() override;

        virtual void useUniformBuffer(const Ref<IUniformBuffer>&) { };
        virtual void useTextureBlock(const std::string& uniformName, const uint8_t& texBlock) { };

        virtual void useMatrix(const std::string& uniformName, const glm::mat4& matrix) { };

        virtual void useVectorf(const std::string& uniformName, const float& x, const float& y) { };
        virtual void useVectorf(const std::string& uniformName,
                                const float& x, const float& y, const float& z) { };
        virtual void useVectorf(const std::string& uniformName,
                                const float& x, const float& y, const float& z, const float& w) { };

        virtual void useVectorf(const std::string& uniformName, const glm::vec2& vec) { };
        virtual void useVectorf(const std::string& uniformName, const glm::vec3& vec) { };
        virtual void useVectorf(const std::string& uniformName, const glm::vec4& vec) { };

        virtual void useFloat(const std::string& uniformName, const float& f) { };
        virtual void useInteger(const std::string& uniformName, const size_t& i) { };

        virtual bool isUniformExists(const std::string& uniformName) { return false; }

        IShader& operator=(const IShader&) noexcept;

    protected:
        std::unordered_map<std::string, IShaderUniform> m_uniforms;

        std::unordered_map<SGShaderDefineType, std::list<ShaderDefine>> m_defines;

    };
    //class IUniformType
}

#endif //NATIVECORE_ISHADER_H
