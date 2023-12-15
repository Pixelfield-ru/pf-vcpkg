#ifndef SUNGEARENGINE_CAMERACOMPONENT_H
#define SUNGEARENGINE_CAMERACOMPONENT_H

#include "SGCore/ECS/Transformations/CameraMovement3DSystem.h"

#include "IRenderingComponent.h"
#include "SGCore/ECS/Rendering/Pipelines/PostProcessFXSubPass.h"

#define SG_PP_LAYER_FB_NAME(idx)  ("frameBuffer" + std::to_string(idx))

namespace SGCore
{
    class IFrameBuffer;

    struct PostProcessLayer
    {
        friend class Camera;

        std::vector<PostProcessFXSubPass> m_subPasses;

        Ref<IFrameBuffer> m_frameBuffer;

        Ref<IShader> m_FXShader;

        // name just for user. for convenience
        std::string m_name = "default";

        std::uint16_t m_index = 0;

        // attachments that will pass the depth test
        std::vector<SGFrameBufferAttachmentType> m_attachmentsToDepthTest { SGG_COLOR_ATTACHMENT1,
                                                                            SGG_COLOR_ATTACHMENT2,
                                                                            SGG_COLOR_ATTACHMENT3,
                                                                            SGG_COLOR_ATTACHMENT4,
                                                                            SGG_COLOR_ATTACHMENT5,
                                                                            SGG_COLOR_ATTACHMENT6 };

        // attachments that the scene will be rendered into
        std::vector<SGFrameBufferAttachmentType> m_attachmentsToRenderIn { SGG_COLOR_ATTACHMENT0,
                                                                           SGG_COLOR_ATTACHMENT1,
                                                                           SGG_COLOR_ATTACHMENT2,
                                                                           SGG_COLOR_ATTACHMENT3,
                                                                           SGG_COLOR_ATTACHMENT4,
                                                                           SGG_COLOR_ATTACHMENT5,
                                                                           SGG_COLOR_ATTACHMENT6 };

        // first - to which attachment of the output buffer will the data from the attachment "second" be copied
        // second - the attachment to be copied
        std::unordered_map<SGFrameBufferAttachmentType, SGFrameBufferAttachmentType> m_attachmentsForCombining {
                { SGG_COLOR_ATTACHMENT0, SGG_COLOR_ATTACHMENT1 },
                { SGG_COLOR_ATTACHMENT1, SGG_COLOR_ATTACHMENT2 },
                { SGG_COLOR_ATTACHMENT2, SGG_COLOR_ATTACHMENT3 },
                { SGG_COLOR_ATTACHMENT3, SGG_COLOR_ATTACHMENT4 },
                { SGG_COLOR_ATTACHMENT4, SGG_COLOR_ATTACHMENT5 },
                { SGG_COLOR_ATTACHMENT5, SGG_COLOR_ATTACHMENT6 }
        };

        std::string getNameInShader() const noexcept
        {
            return m_nameInShader;
        }

    private:
        // technical name
        std::string m_nameInShader = SG_PP_LAYER_FB_NAME(0);
    };

    // todo: make change for default PP shader
    class Camera : public IRenderingComponent
    {
        friend class CameraMovement3DSystem;

    public:
        Camera();

        Ref<IShader> m_depthPassShader;
        Ref<IShader> m_ppLayersCombiningShader;
        Ref<IShader> m_finalPostProcessFXShader;

        Ref<IFrameBuffer> m_ppLayersCombinedBuffer;

        // final frame buffer with all post-processing
        Ref<IFrameBuffer> m_finalFrameFXFrameBuffer;

        // can be helpful for ImGUI
        bool m_useFinalFrameBuffer = false;

        std::set<SGFrameBufferAttachmentType> m_attachmentsForCombining;

        PostProcessLayer& getPostProcessLayer(const Ref<Layer>& layer) noexcept;

        PostProcessLayer& addPostProcessLayer(const std::string& ppLayerName,
                                              const Ref<Layer>& layer,
                                              const std::uint16_t& fbWidth,
                                              const std::uint16_t& fbHeight);

        PostProcessLayer& addPostProcessLayer(const std::string& ppLayerName,
                                              const Ref<Layer>& layer);

        void setPostProcessLayerShader(const Ref<Layer>& layer,
                                       const Ref<IShader>& shader) noexcept;

        const auto& getPostProcessLayers() const noexcept
        {
            return m_postProcessLayers;
        }

        void bindPostProcessFrameBuffer(const Ref<Layer>& layer) noexcept;

        void unbindPostProcessFrameBuffer() const noexcept;

        void clearPostProcessFrameBuffers() const noexcept;

        PostProcessLayer& getDefaultPostProcessLayer() noexcept;

        // todo: make rename pp layer function

    private:
        // uses only for default PP layer
        Ref<Layer> m_technicalLayer = MakeRef<Layer>();

        std::unordered_map<Ref<Layer>, PostProcessLayer> m_postProcessLayers;

        Ref<IFrameBuffer> m_currentPPFrameBufferToBind;

        void init() noexcept final { }
    };
}

#endif //SUNGEARENGINE_CAMERACOMPONENT_H
