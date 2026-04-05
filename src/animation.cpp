#include "engine/animation.h"

namespace AuroraEngine
{

float Animation::get_length() const
{
    return m_frame_timer.get_length();
}

int Animation::current_frame() const
{
    int frame = static_cast<int>(m_frame_timer.get_time() / m_frame_timer.get_length() * m_frame_count);
    return frame % m_frame_count;
}

void Animation::update(float delta_time)
{
    m_frame_timer.update(delta_time);
}

} // namespace AuroraEngine