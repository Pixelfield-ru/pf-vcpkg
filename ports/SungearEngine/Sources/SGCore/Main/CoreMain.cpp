#include "CoreMain.h"

#include <locale>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "SGCore/Graphics/API/GL/GL4/GL4Renderer.h"
#include "SGCore/Graphics/API/GL/GL46/GL46Renderer.h"

#include "SGCore/Memory/AssetManager.h"
#include "SGConsole/API/Console.h"
#include "SGCore/Utils/ShadersPaths.h"

#include "SGCore/Input/InputManager.h"

#include "SGCore/Graphics/API/IRenderer.h"
#include "SGCore/Physics/PhysicsWorld3D.h"
#include "SGCore/UI/FontsManager.h"
#include "SGUtils/CrashHandler/HwExcpetionHandler.h"

void SGCore::CoreMain::start()
{
    const auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::ostringstream timeStringStream;
    timeStringStream << std::put_time(std::localtime(&in_time_t), "%Y_%m_%d_%H_%M_%S");

    const std::string finalLogName = "logs/sg_log_" + timeStringStream.str() + ".log";

    /*CrashHandler::hc_application_name = "Sungear Engine";
    CrashHandler::hc_log_file_output = finalLogName;
    CrashHandler::hc_install();*/

    HwExceptionHandler::setApplicationName("Sungear Engine");
    HwExceptionHandler::setOutputLogFilePath(finalLogName);
    HwExceptionHandler::setupHandler();

    auto currentSessionLogger = spdlog::basic_logger_mt("current_session", finalLogName);
    spdlog::set_default_logger(currentSessionLogger);

    spdlog::flush_on(spdlog::level::info);

    // todo: move
    /*system("chcp 65001");
    setlocale(LC_ALL, "Russian");*/

    m_renderer = GL4Renderer::getInstance();
    // m_renderer = GL46Renderer::getInstance();
    //m_renderer = VkRenderer::getInstance();

    m_window.create();

    m_renderer->init();

    SGUtils::Singleton::getSharedPtrInstance<ShadersPaths>()->createDefaultPaths();
    InputManager::init();
    AssetManager::init();
    FontsManager::init();

    m_renderTimer.onUpdate->connect<&updateStart>(0);
    m_renderTimer.onUpdate->connect<&updateEnd>(std::numeric_limits<size_t>::max());
    m_renderTimer.setTargetFrameRate(1200.0);

    // -----------------
    
    m_fixedTimer.onUpdate->connect<&fixedUpdateStart>(0);
    m_renderTimer.onUpdate->connect<&fixedUpdateEnd>(std::numeric_limits<size_t>::max());
    // m_fixedTimer.m_useFixedUpdateCatchUp = false;

    //Graphics::GL::GL4Renderer::getInstance()->checkForErrors();
    
    (*onInit)();

    m_fixedTimer.resetTimer();
    m_renderTimer.resetTimer();

    // m_fixedTimerThread.detach();

    while (!m_window.shouldClose())
    {
        m_fixedTimer.startFrame();
        m_renderTimer.startFrame();
    }
}

void SGCore::CoreMain::fixedUpdateStart(const double& dt, const double& fixedDt)
{
    InputManager::startFrame();
}

void SGCore::CoreMain::fixedUpdateEnd(const double& dt, const double& fixedDt)
{

}

void SGCore::CoreMain::updateStart(const double& dt, const double& fixedDt)
{
    glm::ivec2 windowSize;
    m_window.getSize(windowSize.x, windowSize.y);
    m_renderer->prepareFrame(windowSize);
}

void SGCore::CoreMain::updateEnd(const double& dt, const double& fixedDt)
{
    m_window.swapBuffers();
    m_window.pollEvents();
}

SGCore::Window& SGCore::CoreMain::getWindow() noexcept
{
    return m_window;
}

SGCore::Ref<SGCore::IRenderer> SGCore::CoreMain::getRenderer() noexcept
{
    return m_renderer;
}

SGCore::Timer& SGCore::CoreMain::getRenderTimer() noexcept
{
    return m_renderTimer;
}

SGCore::Timer& SGCore::CoreMain::getFixedTimer() noexcept
{
    return m_fixedTimer;
}

std::uint16_t SGCore::CoreMain::getFPS() noexcept
{
    return m_renderTimer.getFramesPerSecond();
}