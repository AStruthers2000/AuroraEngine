////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/component.h"

namespace Core
{

//--------------------------------------------------------------------------------------------------
Component::Component(Entity& owning_entity)
    : m_owner(owning_entity)
    , m_order()
{
}

//--------------------------------------------------------------------------------------------------
Component::Component(Entity& owning_entity, Order const& component_order)
    : m_owner(owning_entity)
    , m_order(component_order)
{
}

//--------------------------------------------------------------------------------------------------
Component::~Component() = default;

//--------------------------------------------------------------------------------------------------
void Component::initialize()
{
    initialize_component();
}

//--------------------------------------------------------------------------------------------------
void Component::initialize_component()
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Component::update(float delta_time)
{
    update_component(delta_time);
}

//--------------------------------------------------------------------------------------------------
void Component::update_component(float delta_time)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Component::render(SDL_Renderer* renderer)
{
    render_component(renderer);
}

//--------------------------------------------------------------------------------------------------
void Component::render_component(SDL_Renderer* renderer)
{
    // Intentionally left blank; virtual function.
}

} // namespace Core
