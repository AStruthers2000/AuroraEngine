////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/components/anchor_component.h"

#include "core/entity.h"
#include "core/components/transform_component.h"

namespace Core
{

//--------------------------------------------------------------------------------------------------
glm::vec2 anchor_to_normalized(Anchor anchor)
{
    switch (anchor)
    {
        case Anchor::TopLeft:      return { 0.f, 0.f };
        case Anchor::TopCenter:    return { 0.5f, 0.f };
        case Anchor::TopRight:     return { 1.f, 0.f };
        case Anchor::CenterLeft:   return { 0.f, 0.5f };
        case Anchor::Center:       return { 0.5f, 0.5f };
        case Anchor::CenterRight:  return { 1.f, 0.5f };
        case Anchor::BottomLeft:   return { 0.f, 1.f };
        case Anchor::BottomCenter: return { 0.5f, 1.f };
        case Anchor::BottomRight:  return { 1.f, 1.f };
    }
    return { 0.f, 0.f };
}

//--------------------------------------------------------------------------------------------------
AnchorComponent::AnchorComponent(Entity& owner, Configuration const& config)
    : Component(owner, config)
    , m_self_anchor(config.self_anchor)
    , m_parent_anchor(config.parent_anchor)
    , m_offset(config.offset)
{
}

//--------------------------------------------------------------------------------------------------
void AnchorComponent::on_awake()
{
    m_transform = get_sibling_component<TransformComponent>();

    if (Entity* parent = get_owner().get_parent())
    {
        m_parent_transform = parent->get_component<TransformComponent>("");
    }

    recompute_position();
}

//--------------------------------------------------------------------------------------------------
void AnchorComponent::on_late_update(float delta_time)
{
    recompute_position();
}

//--------------------------------------------------------------------------------------------------
void AnchorComponent::set_self_anchor(Anchor anchor)
{
    m_self_anchor = anchor;
}

//--------------------------------------------------------------------------------------------------
void AnchorComponent::set_parent_anchor(Anchor anchor)
{
    m_parent_anchor = anchor;
}

//--------------------------------------------------------------------------------------------------
void AnchorComponent::set_offset(glm::vec2 offset)
{
    m_offset = offset;
}

//--------------------------------------------------------------------------------------------------
void AnchorComponent::recompute_position()
{
    auto transform = m_transform.lock();
    if (!transform)
    {
        return;
    }

    glm::vec2 parent_size{ 0.f, 0.f };
    if (auto parent_transform = m_parent_transform.lock())
    {
        parent_size = parent_transform->get_size();
    }

    glm::vec2 const parent_point = anchor_to_normalized(m_parent_anchor) * parent_size;
    glm::vec2 const self_pivot   = anchor_to_normalized(m_self_anchor) * (transform->get_size() * transform->get_scale());

    transform->set_position(parent_point + m_offset - self_pivot);
}

} // namespace Core
