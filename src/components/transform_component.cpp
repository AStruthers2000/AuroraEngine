////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/components/transform_component.h"

#include "core/entity.h"

namespace Core
{

//--------------------------------------------------------------------------------------------------
glm::vec2 TransformComponent::get_world_position() const
{
    Entity* parent = get_owner().get_parent();
    if (!parent) return m_local_position;

    auto parent_transform = parent->get_component<TransformComponent>("").lock();
    if (!parent_transform) return m_local_position;

    return m_local_position + parent_transform->get_world_position();
}

//--------------------------------------------------------------------------------------------------
glm::vec2 TransformComponent::get_world_scale() const
{
    Entity* parent = get_owner().get_parent();
    if (!parent) return m_local_scale;

    auto parent_transform = parent->get_component<TransformComponent>("").lock();
    if (!parent_transform) return m_local_scale;

    return m_local_scale * parent_transform->get_world_scale();
}

} // namespace Core
