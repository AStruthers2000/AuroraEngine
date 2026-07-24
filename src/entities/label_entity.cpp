////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/entities/label_entity.h"

#include "core/components/transform_component.h"

namespace Core
{

//--------------------------------------------------------------------------------------------------
LabelEntity::LabelEntity(Layer& owning_layer,
                          glm::vec2 position,
                          std::string_view font_path,
                          int point_size,
                          std::string_view text,
                          SDL_Color color,
                          std::uint8_t update_order)
    : Entity(owning_layer, update_order)
    , m_font_path(font_path)
    , m_point_size(point_size)
    , m_initial_text(text)
    , m_initial_color(color)
{
    // Add the transform in the constructor so it is in the active store before awake() runs.
    add_component<TransformComponent>("", position);
}

//--------------------------------------------------------------------------------------------------
LabelEntity::LabelEntity(Entity& owning_entity,
                          glm::vec2 position,
                          std::string_view font_path,
                          int point_size,
                          std::string_view text,
                          SDL_Color color,
                          std::uint8_t update_order)
    : Entity(owning_entity, update_order)
    , m_font_path(font_path)
    , m_point_size(point_size)
    , m_initial_text(text)
    , m_initial_color(color)
{
    add_component<TransformComponent>("", position);
}

//--------------------------------------------------------------------------------------------------
void LabelEntity::awake()
{
    Component::Order order{};
    m_text_component = add_component<TextRenderComponent>(
        "label",
        order,
        m_font_path,
        m_point_size,
        m_initial_text,
        m_initial_color
    );
}

//--------------------------------------------------------------------------------------------------
void LabelEntity::set_text(std::string_view text)
{
    if (auto ptr = m_text_component.lock())
        ptr->set_text(text);
}

//--------------------------------------------------------------------------------------------------
void LabelEntity::set_color(SDL_Color color)
{
    if (auto ptr = m_text_component.lock())
        ptr->set_color(color);
}

//--------------------------------------------------------------------------------------------------
std::string_view LabelEntity::get_text() const
{
    if (auto ptr = m_text_component.lock())
        return ptr->get_text();
    return {};
}

} // namespace Core
