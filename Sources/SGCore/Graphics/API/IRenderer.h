//
// Created by stuka on 24.04.2023.
//

#ifndef NATIVECORE_IRENDERER_H
#define NATIVECORE_IRENDERER_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/common.hpp>

#include "APIType.h"
#include "IUniformBuffer.h"

namespace Core::ECS
{
    class CameraComponent;
    class MeshComponent;
    class TransformComponent;
    class ShadowsCasterComponent;
    class SkyboxComponent;
    class IRenderingComponent;
    class IPrimitiveComponent;
}

namespace Core::ImportedScene
{
    class IMesh;
}

namespace Core::Graphics
{
    class IVertexBufferLayout;
    class ICubemapTexture;
    class IIndexBuffer;
    class IVertexArray;
    class IVertexBuffer;
    class IFrameBuffer;
    class ITexture2D;
    class IShader;

    // todo: make description of funcs
    class IRenderer
    {
    public:
        // Buffer for storing matrices of the currently main camera.
        std::shared_ptr<IUniformBuffer> m_viewMatricesBuffer;
        // Buffer for storing matrices of the program.
        std::shared_ptr<IUniformBuffer> m_programDataBuffer;

        // check usages in IVertexArray implementations and IIndexBuffer implementations
        // TODO: UB IS HERE
        IIndexBuffer* m_currentBoundIndexBuffer = nullptr;
        IVertexArray* m_currentBoundVertexArray = nullptr;

        virtual void init() { }

        /**
         * Confirmation of GAPI support by the user's graphics card
         */
        virtual bool confirmSupport() noexcept { return false; }

        /**
         *
         * @param windowSize
         */
        virtual void prepareFrame(const glm::ivec2& windowSize) { }

        /**
         * Preparing uniform buffers for render
         * @param renderingComponent - The component that will be used as a "camera" for rendering entities.
         * @param transformComponent - The transform component of this "camera".
         */
        virtual void prepareUniformBuffers(const std::shared_ptr<ECS::IRenderingComponent>& renderingComponent,
                                           const std::shared_ptr<ECS::TransformComponent>& transformComponent) { }

        // TODO: make one method for drawing mesh. input - transformComponent, meshComponent of entity
         /**
          * @param transformComponent - The mesh transformation component to be rendered.
          * @param meshComponent - The mesh component to be rendered.
          */
        // TODO:: remove renderMeshComponent
        virtual void renderMeshComponent(const std::shared_ptr<ECS::TransformComponent>& transformComponent,
                                         const std::shared_ptr<ECS::MeshComponent>& meshComponent) { }

        virtual void renderMesh(const std::shared_ptr<ImportedScene::IMesh>& mesh) { }

        /**
         * Renders a primitive component (i.e. sphere, line, cube, etc.).
         * @param transformComponent - Primitive transformation component.
         * @param primitiveComponent - The component of the primitive to be rendered.
         */
        virtual void renderPrimitiveComponent(const std::shared_ptr<ECS::TransformComponent>& transformComponent,
                                              const std::shared_ptr<ECS::IPrimitiveComponent>& primitiveComponent) { }

        /**
         * Prints information about the graphics capabilities of the kernel on this GAPI and information about the GAPI itself.
         */
        virtual void printInfo() noexcept { }

        virtual void checkForErrors(const std::source_location& = std::source_location::current()) noexcept { }

        [[nodiscard]] virtual IShader* createShader() = 0;
        [[nodiscard]] virtual IShader* createShader(const std::string& path) = 0;

        [[nodiscard]] virtual IVertexBuffer* createVertexBuffer() = 0;
        [[nodiscard]] virtual IVertexArray* createVertexArray() = 0;
        [[nodiscard]] virtual IVertexBufferLayout* createVertexBufferLayout() = 0;
        [[nodiscard]] virtual IIndexBuffer* createIndexBuffer() = 0;
        [[nodiscard]] virtual ITexture2D* createTexture2D() = 0;
        [[nodiscard]] virtual ICubemapTexture* createCubemapTexture() = 0;
        [[nodiscard]] virtual IUniformBuffer* createUniformBuffer() = 0;
        [[nodiscard]] virtual IFrameBuffer* createFrameBuffer() = 0;

        [[nodiscard]] virtual ImportedScene::IMesh* createMesh() = 0;

        // ------------- some settings for renderer ---------
        virtual void setDepthTestingEnabled(const bool& enabled) const noexcept { }
        // --------------------------------------------------

        [[nodiscard]] APIType getAPIType() const noexcept;

    protected:
        APIType m_apiType = SG_API_TYPE_UNKNOWN;
    };
}

#endif //NATIVECORE_IRENDERER_H
