////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/component.h"

#include "core/entity.h"

namespace Core
{

//--------------------------------------------------------------------------------------------------
Component::Component(Entity& owner, Configuration const& config)
    : m_owner(owner)
    , m_order(config.order)
{
}

//--------------------------------------------------------------------------------------------------
Component::~Component() = default;

//--------------------------------------------------------------------------------------------------
std::weak_ptr<Component> Component::get_sibling_component_impl(std::type_index type, std::string_view tag) const
{
    return m_owner.get_component_by_type(type, tag);
}

//--------------------------------------------------------------------------------------------------
void Component::broadcast_event(Event& event)
{
    get_owner().broadcast_event(event);
}

//--------------------------------------------------------------------------------------------------
void Component::broadcast_event_within_entity(Event& event)
{
    get_owner().broadcast_event_within_entity(event);
}

//--------------------------------------------------------------------------------------------------
void Component::awake_component()
{
    on_awake();
}

//--------------------------------------------------------------------------------------------------
void Component::start_component()
{
    on_start();
}

//--------------------------------------------------------------------------------------------------
void Component::update_component(float delta_time)
{
    on_update(delta_time);
}

//--------------------------------------------------------------------------------------------------
void Component::render_component(SDL_Renderer* renderer)
{
    on_render(renderer);
}

//--------------------------------------------------------------------------------------------------
void Component::late_update_component(float delta_time)
{
    on_late_update(delta_time);
}

//--------------------------------------------------------------------------------------------------
void Component::fixed_update_component(float fixed_dt)
{
    on_fixed_update(fixed_dt);
}

//--------------------------------------------------------------------------------------------------
void Component::cleanup_component()
{
    on_cleanup();
}

} // namespace Core
