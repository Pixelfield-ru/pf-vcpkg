//
// Created by stuka on 25.11.2023.
//

#include "PBRFRPGeometryPass.h"
#include "SGCore/ECS/ECSUtils.h"
#include "SGCore/Main/CoreMain.h"
#include "SGCore/ECS/Rendering/Camera.h"
#include "SGCore/ECS/Rendering/Mesh.h"
#include "PBRFRPShadowsPass.h"
#include "SGCore/ECS/Rendering/Lighting/ShadowsCaster.h"

void SGCore::PBRFRPGeometryPass::render(const Ref<Scene>& scene, const SGCore::Ref<SGCore::IRenderPipeline>& renderPipeline)
{
    Ref<PBRFRPShadowsPass> shadowsPass = renderPipeline->getRenderPass<PBRFRPShadowsPass>();

    m_shader->bind();
    m_shader->useShaderMarkup(m_shaderMarkup);

    if(shadowsPass || !shadowsPass->m_active)
    {
        const auto& shadowsMapsTexturesBlock =
                m_shaderMarkup.m_texturesBlocks[SGTextureType::SGTP_SHADOW_MAP];

        std::uint8_t currentShadowsCaster = 0;

        SG_BEGIN_ITERATE_CACHED_ENTITIES(*shadowsPass->m_componentsToRenderIn, shadowsCastersLayer,
                                         shadowsCasterEntity)
                // todo: make process all ShadowsCasterComponent (cachedEntities.second->getComponents)
                auto shadowsCaster = shadowsCasterEntity.getComponent<ShadowsCaster>();

                if(!shadowsCaster) continue;

                shadowsCaster->m_frameBuffer->bindAttachment(
                        SGFrameBufferAttachmentType::SGG_DEPTH_ATTACHMENT0,
                        shadowsMapsTexturesBlock.m_offset + currentShadowsCaster
                );

                currentShadowsCaster++;
        SG_END_ITERATE_CACHED_ENTITIES
    }

    SG_BEGIN_ITERATE_CACHED_ENTITIES(*m_componentsToRenderIn, camerasLayer, cameraEntity)
            auto cameraTransformComponent = cameraEntity.getComponent<Transform>();
            if(!cameraTransformComponent) continue;
            auto cameraComponent = cameraEntity.getComponent<Camera>();
            if(!cameraComponent) continue;

            CoreMain::getRenderer().prepareUniformBuffers(cameraComponent, cameraTransformComponent);
            m_shader->useUniformBuffer(CoreMain::getRenderer().m_viewMatricesBuffer);

            // todo: make less bindings

            for(auto& meshesLayer : *m_componentsToRender)
            {
                const auto& layer = meshesLayer.first;

                cameraComponent->bindPostProcessFrameBuffer(layer,SGG_COLOR_ATTACHMENT0);

                for(auto& meshesEntity: meshesLayer.second)
                {
                    Ref<Transform> transformComponent = meshesEntity.second.getComponent<Transform>();

                    if(!transformComponent) continue;

                    auto meshComponents =
                            meshesEntity.second.getComponents<Mesh>();

                    for(const auto& meshComponent: meshComponents)
                    {
                        meshComponent->m_meshData->m_material->bind(m_shader, m_shaderMarkup);
                        m_shader->useMatrix("objectModelMatrix",
                                            transformComponent->m_modelMatrix
                        );

                        CoreMain::getRenderer().renderMeshData(
                                meshComponent->m_meshData,
                                meshComponent->m_meshDataRenderInfo
                        );
                    }
                }

                cameraComponent->unbindPostProcessFrameBuffer();
            }
    SG_END_ITERATE_CACHED_ENTITIES
}
