////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/entities/label.h"

#include "core/components/transform_component.h"

namespace Core::UI
{

//--------------------------------------------------------------------------------------------------
Label::Label(Entity::Owner owner, Configuration const& config)
    : Entity(owner, config)
    , m_font_path(config.font_path)
    , m_point_size(config.point_size)
    , m_initial_text(config.text)
    , m_initial_color(config.color)
{
    add_component<TransformComponent>("", { .position = config.position });
}

//--------------------------------------------------------------------------------------------------
void Label::awake()
{
    // Component::Order order{};
    // m_text_component = add_component<TextRenderComponent>(
    //     "label",
    //     order,
    //     m_font_path,
    //     m_point_size,
    //     m_initial_text,
    //     m_initial_color
    // );
    TextRenderComponent::Configuration config
    {
        .font_path = m_font_path,
        .point_size = m_point_size,
        .text = m_initial_text,
        .color = m_initial_color,
    };
    m_text_component = add_component<TextRenderComponent>("label", config);
}

//--------------------------------------------------------------------------------------------------
void Label::set_text(std::string_view text)
{
    if (auto ptr = m_text_component.lock())
        ptr->set_text(text);
}

//--------------------------------------------------------------------------------------------------
void Label::set_color(SDL_Color color)
{
    if (auto ptr = m_text_component.lock())
        ptr->set_color(color);
}

//--------------------------------------------------------------------------------------------------
std::string_view Label::get_text() const
{
    if (auto ptr = m_text_component.lock())
        return ptr->get_text();
    return {};
}

} // namespace Core
