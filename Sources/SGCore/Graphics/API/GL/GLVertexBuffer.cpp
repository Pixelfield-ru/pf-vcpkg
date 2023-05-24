//
// Created by stuka on 23.05.2023.
//

#include "GLVertexBuffer.h"
#include "SGCore/Graphics/API/GL/GL46/GL46Renderer.h"

Core::Graphics::API::GL::GLVertexBuffer::~GLVertexBuffer() noexcept
{
    destroy();
}

Core::Graphics::API::GL::GLVertexBuffer* Core::Graphics::API::GL::GLVertexBuffer::create() noexcept
{
    destroy();

    glGenBuffers(1, &m_handler);

    #ifdef SUNGEAR_DEBUG
    GL46::GL46Renderer::getInstance()->checkForErrors();
    #endif

    return this;
}

void Core::Graphics::API::GL::GLVertexBuffer::destroy() noexcept
{
    glDeleteBuffers(1, &m_handler);

    #ifdef SUNGEAR_DEBUG
    GL46::GL46Renderer::getInstance()->checkForErrors();
    #endif
}

Core::Graphics::API::GL::GLVertexBuffer* Core::Graphics::API::GL::GLVertexBuffer::putData
(std::vector<float> data) noexcept
{
    m_data = std::move(data);

    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (m_data.size() * sizeof(GLsizeiptr)), &m_data[0],
                 castSGGBufferUsageToOGL(m_usage));

    #ifdef SUNGEAR_DEBUG
    GL46::GL46Renderer::getInstance()->checkForErrors();
    #endif

    return this;
}

Core::Graphics::API::GL::GLVertexBuffer* Core::Graphics::API::GL::GLVertexBuffer::subData
(std::vector<float> data, const int& offset) noexcept
{
    glBufferSubData(GL_ARRAY_BUFFER, offset, (GLsizeiptr) (m_data.size() * sizeof(GLsizeiptr)),
                    (const void*) &m_data[0]);

    #ifdef SUNGEAR_DEBUG
    GL46::GL46Renderer::getInstance()->checkForErrors();
    #endif

    return this;
}

Core::Graphics::API::GL::GLVertexBuffer* Core::Graphics::API::GL::GLVertexBuffer::bind() noexcept
{
    glBindBuffer(GL_ARRAY_BUFFER, m_handler);

    #ifdef SUNGEAR_DEBUG
    GL46::GL46Renderer::getInstance()->checkForErrors();
    #endif

    return this;
}

GLenum Core::Graphics::API::GL::GLVertexBuffer::castSGGBufferUsageToOGL(const SGGBufferUsage& sggBufferUsage) noexcept
{
    GLenum usage = GL_STATIC_DRAW;

    switch(sggBufferUsage)
    {
        case SGG_BUFFER_USAGE_DYNAMIC: usage = GL_DYNAMIC_DRAW; break;
        case SGG_BUFFER_USAGE_STATIC: usage = GL_STATIC_DRAW; break;
    }

    return usage;
}

Core::Graphics::API::GL::GLVertexBuffer* Core::Graphics::API::GL::GLVertexBuffer::setUsage(SGGBufferUsage usage) noexcept
{
    m_usage = usage;

    return this;
}
