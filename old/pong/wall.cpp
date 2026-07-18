#include "wall.h"

#include "core/components/render_components/rect_render_component.h"

Wall::Wall(Core::Layer& owner, std::uint8_t update_order, glm::vec2 position, glm::vec2 scale)
    : Core::Entity(owner, update_order)
{
    add_component<Core::TransformComponent>(position, scale);
    add_component<Core::RectRenderComponent>(m_component_order, m_color);
}
