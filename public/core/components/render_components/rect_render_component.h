////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Renders a rectangle to the screen during the render phase of each frame.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_COMPONENTS_RENDER_COMPONENTS_RECT_RENDER_COMPONENT_H
#define CORE_COMPONENTS_RENDER_COMPONENTS_RECT_RENDER_COMPONENT_H

#include "core/components/render_component.h"

namespace Core
{

class TransformComponent;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Renders a filled rectangle to the screen during the render phase of each frame.
////////////////////////////////////////////////////////////////////////////////////////////////////
class RectRenderComponent : public RenderComponent
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief The rectangle render component renders a filled rectangle to the screen during the
    ///        render phase of each frame. The size and position of the rectangle are determined by
    ///        the sibling TransformComponent.
    ///
    /// @param [in] owning_entity   - The Entity that owns this Component instance. Passed to parent
    ///                               constructor.
    /// @param [in] component_order - The sorting order mapping this Component will follow. Passed
    ///                               to parent constructor.
    /// @param [in] rect_color      - Color of the filled rectangle.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    RectRenderComponent(Entity& owning_entity, Order const& component_order, SDL_Color rect_color);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Initializes this render component by getting a reference to the sibling
    ///        TransformComponent.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void awake() override;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Renders a filled rectangle to the screen. Uses the sibling TransformComponent to
    ///        determine position/scale.
    ///
    /// @param [in] renderer - Renderer provided by the owning Entity. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void render(SDL_Renderer* renderer) override;

private:
    SDL_Color m_color{};
    std::weak_ptr<TransformComponent> m_owning_transform{};
};

} // namespace Core

# endif // CORE_COMPONENTS_RENDER_COMPONENTS_RECT_RENDER_COMPONENT_H
