////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/component.h"

#include "core/entity.h"

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
std::weak_ptr<Component> Component::get_sibling_component_impl(std::type_index type) const
{
    return m_owner.get_component_by_type(type);
}

//--------------------------------------------------------------------------------------------------
void Component::awake_component()
{
    awake();
}

//--------------------------------------------------------------------------------------------------
void Component::awake()
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Component::start_component()
{
    start();
}

//--------------------------------------------------------------------------------------------------
void Component::start()
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Component::update_component(float delta_time)
{
    update(delta_time);
}

//--------------------------------------------------------------------------------------------------
void Component::update(float delta_time)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Component::render_component(SDL_Renderer* renderer)
{
    render(renderer);
}

//--------------------------------------------------------------------------------------------------
void Component::render(SDL_Renderer* renderer)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Component::late_update_component(float delta_time)
{
    late_update(delta_time);
}

//--------------------------------------------------------------------------------------------------
void Component::late_update(float delta_time)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Component::fixed_update_component(float fixed_dt)
{
    fixed_update(fixed_dt);
}

//--------------------------------------------------------------------------------------------------
void Component::fixed_update(float fixed_dt)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Component::cleanup_component()
{
    cleanup();
}

//--------------------------------------------------------------------------------------------------
void Component::cleanup()
{
    // Intentionally left blank; virtual function.
}

} // namespace Core
