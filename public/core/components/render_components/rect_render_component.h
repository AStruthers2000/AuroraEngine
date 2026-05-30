////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_COMPONENTS_RENDER_COMPONENTS_RECT_RENDER_COMPONENT_H
#define CORE_COMPONENTS_RENDER_COMPONENTS_RECT_RENDER_COMPONENT_H

#include "core/components/render_component.h"

namespace Core
{

class TransformComponent;

class RectRenderComponent : public RenderComponent
{
public:
    RectRenderComponent(Entity& owning_entity, Order const& component_order, SDL_Color rect_color);

    virtual void awake_component() override;
    virtual void render_component(SDL_Renderer* renderer) override;

private:
    SDL_Color m_color{};
    std::weak_ptr<TransformComponent> m_owning_transform{};
};

} // namespace Core

# endif // CORE_COMPONENTS_RENDER_COMPONENTS_RECT_RENDER_COMPONENT_H
