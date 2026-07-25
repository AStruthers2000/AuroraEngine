////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/components/transform_component.h"

#include "core/entity.h"

namespace Core
{

TransformComponent::TransformComponent(Entity& owner, Configuration const& config)
    : Component(owner, config)
    , m_position(config.position)
    , m_velocity(config.velocity)
    , m_scale(config.scale)
    , m_size(config.size)
{
}

//--------------------------------------------------------------------------------------------------
glm::vec2 TransformComponent::get_world_position() const
{
    Entity* parent = get_owner().get_parent();
    if (!parent) return m_position;

    auto parent_transform = parent->get_component<TransformComponent>("").lock();
    if (!parent_transform) return m_position;

    return m_position + parent_transform->get_world_position();
}

//--------------------------------------------------------------------------------------------------
glm::vec2 TransformComponent::get_world_scale() const
{
    Entity* parent = get_owner().get_parent();
    if (!parent) return m_scale;

    auto parent_transform = parent->get_component<TransformComponent>("").lock();
    if (!parent_transform) return m_scale;

    return m_scale * parent_transform->get_world_scale();
}

} // namespace Core
