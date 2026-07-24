////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief A self-contained entity that renders a single line of text. Intended to be used as
///        either a root entity on a Layer or as a child entity of a composite UI entity.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_ENTITIES_LABEL_ENTITY_H
#define CORE_ENTITIES_LABEL_ENTITY_H

#include "core/entity.h"
#include "core/components/render_components/text_render_component.h"

#include <string>
#include <string_view>

namespace Core
{

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief A self-contained entity that renders a single line of text. Intended to be used as
///        either a root entity on a Layer or as a child entity of a composite UI entity.
////////////////////////////////////////////////////////////////////////////////////////////////////
class LabelEntity : public Entity
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Constructs a LabelEntity as a root entity on a Layer.
    ///
    /// @param [in] owning_layer  - The Layer that owns this entity.
    /// @param [in] position      - World position of the label.
    /// @param [in] font_path     - Path to the font file. Use a constant from Core::Font.
    /// @param [in] point_size    - Font size in points.
    /// @param [in] text          - Initial text content.
    /// @param [in] color         - Text color.
    /// @param [in] update_order  - Optional update order (default: DEFAULT_SORTING_ORDER).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    LabelEntity(Layer& owning_layer,
                glm::vec2 position,
                std::string_view font_path,
                int point_size,
                std::string_view text,
                SDL_Color color,
                std::uint8_t update_order = DEFAULT_SORTING_ORDER);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Constructs a LabelEntity as a child entity of another Entity.
    ///
    /// @param [in] owning_entity - The parent Entity.
    /// @param [in] position      - Local position of the label (offset from parent's world pos).
    /// @param [in] font_path     - Path to the font file. Use a constant from Core::Font.
    /// @param [in] point_size    - Font size in points.
    /// @param [in] text          - Initial text content.
    /// @param [in] color         - Text color.
    /// @param [in] update_order  - Optional update order.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    LabelEntity(Entity& owning_entity,
                glm::vec2 position,
                std::string_view font_path,
                int point_size,
                std::string_view text,
                SDL_Color color,
                std::uint8_t update_order = DEFAULT_SORTING_ORDER);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Adds the TextRenderComponent. Called automatically during initialization.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void awake() override;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Sets the displayed text at runtime.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_text(std::string_view text);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Sets the text color at runtime.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_color(SDL_Color color);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The current text string.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::string_view get_text() const;

private:
    // Construction-time parameters, held until awake() wires up the component weak_ptr
    std::string m_font_path;
    int m_point_size;
    std::string m_initial_text;
    SDL_Color m_initial_color;

    std::weak_ptr<TextRenderComponent> m_text_component{};
};

} // namespace Core

#endif // CORE_ENTITIES_LABEL_ENTITY_H
