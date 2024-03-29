//
// Created by stuka on 27.07.2023.
//

#ifndef SUNGEARENGINE_VKFRAMEBUFFER_H
#define SUNGEARENGINE_VKFRAMEBUFFER_H

#include "SGCore/Graphics/API/IFrameBuffer.h"

namespace SGCore
{
    class VkFrameBuffer : public IFrameBuffer
    {
    public:
        std::shared_ptr<IFrameBuffer> bind() override;
        std::shared_ptr<IFrameBuffer> unbind() override;

        std::shared_ptr<IFrameBuffer> create() override;
        void destroy() override;

        std::shared_ptr<IFrameBuffer> addAttachment(SGFrameBufferAttachmentType,
                                                    SGGColorFormat format, SGGColorInternalFormat internalFormat,
                                                    const int& mipLevel,
                                                    const int& layer) override;
    };
}

#endif //SUNGEARENGINE_VKFRAMEBUFFER_H
