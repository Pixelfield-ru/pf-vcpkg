#include "GL4Renderer.h"

#include "SGCore/Main/CoreMain.h"

#include <glm/gtc/type_ptr.hpp>

#include <thread>

#include "SGCore/ECS/Transformations/TransformComponent.h"
#include "SGCore/ECS/Rendering/MeshComponent.h"
#include "SGCore/ECS/Rendering/CameraComponent.h"
#include "SGCore/ECS/Rendering/Lighting/ShadowsCasterComponent.h"
#include "SGCore/ECS/Rendering/SkyboxComponent.h"

#include "SGCore/Graphics/API/GL/GLGraphicsTypesCaster.h"

void Core::Graphics::GL4Renderer::init() noexcept
{
    SGCF_INFO("-----------------------------------", SG_LOG_CURRENT_SESSION_FILE);
    SGCF_INFO("GLRenderer initializing...", SG_LOG_CURRENT_SESSION_FILE);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        SGCF_ERROR("Failed to initialize GLRenderer.", SG_LOG_CURRENT_SESSION_FILE);
    }
    else
    {
        SGCF_INFO("GLRenderer initialized!", SG_LOG_CURRENT_SESSION_FILE);
    }

    printInfo();
    SGCF_INFO("-----------------------------------", SG_LOG_CURRENT_SESSION_FILE);

    // -------------------------------------

    if(!confirmSupport())
    {
        Core::Main::CoreMain::getWindow().setShouldClose(true);
    }

    setDepthTestingEnabled(true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    /*glEnable (GL_ALPHA_TEST);
    glAlphaFunc (GL_GREATER, 0.2);*/

    /*glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);*/

    // -------------------------------------

    // TODO: make defines for uniforms names

    m_viewMatricesBuffer = std::shared_ptr<GL4UniformBuffer>(createUniformBuffer());
    m_viewMatricesBuffer->m_blockName = "ViewMatrices";
    m_viewMatricesBuffer->putUniforms({
        Core::Graphics::IShaderUniform("projectionMatrix", SGGDataType::SGG_MAT4),
        Core::Graphics::IShaderUniform("viewMatrix", SGGDataType::SGG_MAT4),
        Core::Graphics::IShaderUniform("viewDirection", SGGDataType::SGG_FLOAT3)
                                        });
    m_viewMatricesBuffer->putData<float>({ });
    m_viewMatricesBuffer->putData<float>({ });
    m_viewMatricesBuffer->putData<float>({ });
    m_viewMatricesBuffer->setLayoutLocation(1);
    m_viewMatricesBuffer->prepare();

    m_programDataBuffer = std::shared_ptr<GL4UniformBuffer>(createUniformBuffer());
    m_programDataBuffer->m_blockName = "ProgramData";
    m_programDataBuffer->putUniforms({
                                               Core::Graphics::IShaderUniform("windowSize", SGGDataType::SGG_MAT4)
                                       });
    m_programDataBuffer->putData<float>({ });
    m_programDataBuffer->setLayoutLocation(3);
    m_programDataBuffer->prepare();
}

bool Core::Graphics::GL4Renderer::confirmSupport() noexcept
{
    std::string glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    // because gl version always has this pattern
    int glMajorVersion = glVersion[0] - '0';
    int glMinorVersion = glVersion[2] - '0';

    if(glMajorVersion < 4)
    {
        SGCF_ERROR("OpengGL 4.0 is not supported!", SG_LOG_CURRENT_SESSION_FILE);

        return false;
    }

    return true;
}

void Core::Graphics::GL4Renderer::checkForErrors(const std::source_location& location) noexcept
{
    int errCode = glGetError();

    std::string errStr;

    switch(errCode)
    {
        case GL_INVALID_ENUM: errStr = "GL_INVALID_ENUM: enumeration parameter is not a legal enumeration for that function."; break;
        case GL_INVALID_VALUE: errStr = "GL_INVALID_VALUE: value parameter is not a legal value for that function."; break;
        case GL_INVALID_OPERATION: errStr = "GL_INVALID_OPERATION: set of state for a command is not legal for the parameters given to that command."; break;
        case GL_STACK_OVERFLOW: errStr = "GL_STACK_OVERFLOW: stack pushing operation cannot be done because it would overflow the limit of that stack's size."; break;
        case GL_STACK_UNDERFLOW: errStr = "GL_STACK_UNDERFLOW: stack popping operation cannot be done because the stack is already at its lowest point."; break;
        case GL_OUT_OF_MEMORY: errStr = "GL_OUT_OF_MEMORY: performing an operation that can allocate memory, and the memory cannot be allocated."; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: errStr = "GL_INVALID_FRAMEBUFFER_OPERATION: doing anything that would attempt to read from or write/render to a framebuffer that is not complete."; break;
        case GL_CONTEXT_LOST: errStr = "GL_CONTEXT_LOST: the OpenGL context has been lost, due to a graphics card reset."; break;
        default: errStr = "Unknown error"; break;
    };

    if(errCode != 0)
    {
        SGC_ERROR_SL("OpenGL error (code: " + std::to_string(errCode) + "): " + errStr, location);
    }
}

void Core::Graphics::GL4Renderer::printInfo() noexcept
{
    SGCF_INFO("GLRenderer info:", SG_LOG_CURRENT_SESSION_FILE);
    SGCF_INFO("OpenGL version is " + std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))), SG_LOG_CURRENT_SESSION_FILE);
    SGF_INFO("Supporting extensions: ", SG_GL_SUPPORTING_EXTENSIONS_FILE);

    GLint extensionsNum = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extensionsNum);

    for(int i = 0; i < extensionsNum; i++)
    {
        SGF_INFO(std::string(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i))), SG_GL_SUPPORTING_EXTENSIONS_FILE);
    }
}

void Core::Graphics::GL4Renderer::prepareFrame(const glm::ivec2& windowSize)
{
    glViewport(0, 0, windowSize.x, windowSize.y);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0, 0, 0, 0);
}

void Core::Graphics::GL4Renderer::prepareUniformBuffers(const std::shared_ptr<ECS::IRenderingComponent>& renderingComponent,
                                                        const std::shared_ptr<ECS::TransformComponent>& transformComponent)
{
    m_viewMatricesBuffer->bind();
    m_programDataBuffer->bind();

    if(renderingComponent)
    {
        m_viewMatricesBuffer->subData("viewMatrix",
                                      glm::value_ptr(renderingComponent->m_viewMatrix), 16);
        m_viewMatricesBuffer->subData("projectionMatrix",
                                      glm::value_ptr(renderingComponent->m_projectionMatrix), 16);
    }
    if(transformComponent)
    {
        m_viewMatricesBuffer->subData("viewDirection",
                                      glm::value_ptr(transformComponent->m_position), 3);
    }

    int windowWidth;
    int windowHeight;

    Main::CoreMain::getWindow().getSize(windowWidth, windowHeight);
    // todo: перенести обновление в класс окна
    m_programDataBuffer->subData("windowSize", { windowWidth, windowHeight });
}

void Core::Graphics::GL4Renderer::renderMesh(
        const std::shared_ptr<ECS::TransformComponent>& transformComponent,
        const std::shared_ptr<ECS::MeshComponent>& meshComponent
        )
{
    if(!meshComponent->m_mesh) return;

    // const auto& materialShader = meshComponent->m_mesh->m_material->getCurrentShader();

    // if(!materialShader) return;

    if(meshComponent->m_enableFacesCulling)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GLGraphicsTypesCaster::sggFaceTypeToGL(meshComponent->m_facesCullingFaceType));
        glFrontFace(GLGraphicsTypesCaster::sggPolygonsOrderToGL(
                meshComponent->m_facesCullingPolygonsOrder)
        );
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    // meshComponent->m_mesh->m_material->bind();
    meshComponent->m_mesh->getVertexArray()->bind();

    // materialShader->useUniformBuffer(m_viewMatricesBuffer);
    // materialShader->useUniformBuffer(m_programDataBuffer);

    glDrawElements(GLGraphicsTypesCaster::sggDrawModeToGL(meshComponent->m_mesh->m_drawMode), meshComponent->m_mesh->getVertexArray()->m_indicesCount, GL_UNSIGNED_INT, nullptr);
}

void Core::Graphics::GL4Renderer::renderRenderPass(RenderPass& renderPass,
                                                   const std::shared_ptr<ImportedScene::IMesh>& mesh)
{
    /*if(!mesh) return;

    const auto& materialShader = mesh->m_material->getCurrentShader();

    if(!materialShader) return;

    mesh->m_material->bind();
    std::uint8_t frameBufferIndex = 0;
    for(const auto& ppLayer : renderPass.m_postProcessLayers)
    {
        const auto& frameBuffer = ppLayer.second;

        frameBuffer->bindAttachments(mesh->m_material, frameBufferIndex);
        frameBufferIndex++;
    }

    renderPass.m_defaultLayerFrameBuffer->bindAttachments(mesh->m_material, frameBufferIndex);
    // frameBuffer->bindAttachments(mesh->m_material);
    mesh->getVertexArray()->bind();

    materialShader->useUniformBuffer(m_viewMatricesBuffer);
    materialShader->useUniformBuffer(m_programDataBuffer);

    glDrawElements(GLGraphicsTypesCaster::sggDrawModeToGL(mesh->m_drawMode),
                   mesh->getVertexArray()->m_indicesCount, GL_UNSIGNED_INT, nullptr);*/
}

void Core::Graphics::GL4Renderer::renderPrimitive(const std::shared_ptr<ECS::TransformComponent>& transformComponent,
                                                  const std::shared_ptr<ECS::IPrimitiveComponent>& primitiveComponent)
{
    // const auto& materialShader = primitiveComponent->m_mesh->m_material->getCurrentShader();

    // if(!materialShader) return;

    // primitiveComponent->m_mesh->m_material->bind();
    if(primitiveComponent->m_mesh->getVertexArray())
    {
        primitiveComponent->m_mesh->getVertexArray()->bind();
    }

    //materialShader->useUniformBuffer(m_modelMatricesBuffer);
    // materialShader->useUniformBuffer(m_viewMatricesBuffer);

    glLineWidth(primitiveComponent->m_linesWidth);
    //glPointSize(primitiveComponent->m_linesWidth);

    if(!primitiveComponent->m_mesh->m_useIndices)
    {
        glDrawArrays(GL_LINES, 0, primitiveComponent->m_mesh->m_positions.size());
    }
    else
    {
        glDrawElements(GL_LINES, primitiveComponent->m_mesh->getVertexArray()->m_indicesCount,
                       GL_UNSIGNED_INT, nullptr);
    }
}

Core::Graphics::GL46Shader* Core::Graphics::GL4Renderer::createShader()
{
    auto* shader = new GL46Shader;
    shader->m_version = "400";

    return shader;
}

Core::Graphics::GL46Shader* Core::Graphics::GL4Renderer::createShader(const std::string& path)
{
    auto* shader = createShader();

    shader->compile(
            Core::Memory::AssetManager::loadAsset<Core::Memory::Assets::FileAsset>(path)
    );

    return shader;
}

Core::Graphics::GLVertexArray* Core::Graphics::GL4Renderer::createVertexArray()
{
    return new GLVertexArray;
}

Core::Graphics::GLVertexBuffer* Core::Graphics::GL4Renderer::createVertexBuffer()
{
    return new GLVertexBuffer;
}

Core::Graphics::GLVertexBufferLayout* Core::Graphics::GL4Renderer::createVertexBufferLayout()
{
    return new GLVertexBufferLayout;
}

Core::Graphics::GLIndexBuffer* Core::Graphics::GL4Renderer::createIndexBuffer()
{
    return new GLIndexBuffer;
}

Core::Graphics::GL4Texture2D* Core::Graphics::GL4Renderer::createTexture2D()
{
    return new GL4Texture2D;
}

Core::Graphics::GL4CubemapTexture* Core::Graphics::GL4Renderer::createCubemapTexture()
{
    return new GL4CubemapTexture;
}

Core::Graphics::GL4UniformBuffer* Core::Graphics::GL4Renderer::createUniformBuffer()
{
    return new GL4UniformBuffer;
}

Core::Graphics::GL4FrameBuffer* Core::Graphics::GL4Renderer::createFrameBuffer()
{
    return new GL4FrameBuffer;
}

Core::Graphics::GL3Mesh* Core::Graphics::GL4Renderer::createMesh()
{
    return new GL3Mesh;
}

void Core::Graphics::GL4Renderer::setDepthTestingEnabled(const bool& enabled) const noexcept
{
    if(enabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
}

const std::shared_ptr<Core::Graphics::GL4Renderer>& Core::Graphics::GL4Renderer::getInstance() noexcept
{
    static std::shared_ptr<GL4Renderer> s_instancePointer(new GL4Renderer);
    s_instancePointer->m_apiType = SG_API_TYPE_GL4;

    return s_instancePointer;
}
