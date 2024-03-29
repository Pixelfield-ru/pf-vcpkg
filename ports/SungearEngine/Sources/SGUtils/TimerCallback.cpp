//
// Created by stuka on 06.05.2023.
//

#include "TimerCallback.h"

void SGCore::TimerCallback::setStartFunction(std::function<void()>&& function) noexcept
{
    m_startFunction = function;
}

void SGCore::TimerCallback::setUpdateFunction(std::function<void(const double& dt, const double& fixedDt)>&& function) noexcept
{
    m_updateFunction = function;
}

// ------------------------------------------------------------------

void SGCore::TimerCallback::callStartFunction()
{
    if(m_startFunction)
    {
        m_startFunction();
    }
}

void SGCore::TimerCallback::callUpdateFunction(const double& dt, const double& fixedDt)
{
    if(m_updateFunction)
    {
        m_updateFunction(dt, fixedDt);
    }
}
