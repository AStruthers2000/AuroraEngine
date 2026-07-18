////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_EVENTS_EVENT_DISPATCHER_H
#define CORE_EVENTS_EVENT_DISPATCHER_H

#include "event.h"

#include <functional>

namespace Core
{

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class EventDispatcher
{
    template<typename TEvent>
    using EventFn = std::function<bool(TEvent&)>;

public:
    EventDispatcher(Event& event)
        : m_event(event) {}

    template<typename TEvent>
    requires(std::derived_from<TEvent, Event>)
    bool dispatch(EventFn<TEvent> func)
    {
        bool const is_matching_event = m_event.get_event_type() == TEvent::get_static_type();
        bool const is_event_handled  = m_event.get_handled();
        if (is_matching_event && !is_event_handled)
        {
            bool const now_handled = func(static_cast<TEvent&>(m_event));
            m_event.set_handled(now_handled);
            return true;
        }
        return false;
    }

private:
    Event& m_event;
};

} // namespace Core

#endif // CORE_EVENTS_EVENT_DISPATCHER_H
