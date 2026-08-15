////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/entities/ui/ui_element.h"

namespace Core::UI
{

//--------------------------------------------------------------------------------------------------
UIElement::UIElement(Entity::Owner owner, Configuration const& config)
    : Entity(owner, config)
{
    add_component<AnchorComponent>("", {
        .self_anchor = config.self_anchor,
        .parent_anchor = config.parent_anchor,
        .offset = config.offset,
    });
}

//--------------------------------------------------------------------------------------------------
void UIElement::set_self_anchor(Anchor anchor)
{
    if (auto anchor_ptr = m_anchor.lock())
    {
        anchor_ptr->set_self_anchor(anchor);
    }
}

//--------------------------------------------------------------------------------------------------
void UIElement::set_parent_anchor(Anchor anchor)
{
    if (auto anchor_ptr = m_anchor.lock())
    {
        anchor_ptr->set_parent_anchor(anchor);
    }
}

//--------------------------------------------------------------------------------------------------
void UIElement::set_offset(glm::vec2 offset)
{
    if (auto anchor_ptr = m_anchor.lock())
    {
        anchor_ptr->set_offset(offset);
    }
}

} // namespace Core::UI
