//
// Created by stuka on 03.02.2024.
//

#include "DirectionalLight.h"
#include "SGCore/Main/CoreMain.h"
#include "SGCore/Graphics/API/IFrameBuffer.h"
#include "SGCore/Graphics/API/IRenderer.h"
#include "SGCore/Graphics/API/GraphicsDataTypes.h"

SGCore::DirectionalLight::DirectionalLight() noexcept
{
    m_base.m_shadowMap = Ref<IFrameBuffer>(CoreMain::getRenderer()->createFrameBuffer())
            ->create()
            ->bind()
            ->setSize(1024 * 2, 1024 * 2)
            ->addAttachment(SGFrameBufferAttachmentType::SGG_DEPTH_ATTACHMENT0,
                            SGGColorFormat::SGG_DEPTH_COMPONENT,
                            SGGColorInternalFormat::SGG_DEPTH_COMPONENT32F,
                            0,
                            0)
            ->unbind();
}
