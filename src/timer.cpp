#include "engine/timer.h"

namespace AuroraEngine
{

void Timer::update(float delta_time)
{
    m_time += delta_time;
    if (m_time >= m_length)
    {
        m_time -= m_length;
        m_timeout = true;
    }
}

void Timer::reset()
{
    m_time = 0.0f;
    m_timeout = false;
}

} // namespace AuroraEngine
