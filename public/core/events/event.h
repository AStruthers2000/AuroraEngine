////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_EVENTS_EVENT_H
#define CORE_EVENTS_EVENT_H

#include <string>
#include <typeindex>

namespace Core
{

#define EVENT_CLASS_TYPE(TypeName) static std::type_index get_static_type() { return std::type_index(typeid(TypeName)); }\
                                   virtual std::type_index get_event_type() const override { return get_static_type(); }\
                                   virtual char const* get_name() const override { return #TypeName; }

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class Event
{
public:
    virtual ~Event() {}
    virtual std::type_index get_event_type() const = 0;
    virtual char const* get_name() const = 0;
    virtual std::string to_string() const { return get_name(); }
    bool get_handled() const { return m_handled; }
    void set_handled(bool handled) { m_handled = handled; }

private:
    bool m_handled{ false };
};

} // namespace Core

#endif // CORE_EVENTS_EVENT_H
