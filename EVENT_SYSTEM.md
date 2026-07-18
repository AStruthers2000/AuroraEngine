# Event System — Type-Erased Redesign

## Overview

The current event system identifies event types through a hand-maintained `EventType` enum.
Every new event type requires modifying that enum in the engine. Custom user-defined events
cannot be added without touching engine source, and two events that share the same enum value
(e.g. `EventType::None`) are indistinguishable at dispatch time — `EventDispatcher::dispatch<T>()`
would incorrectly match both.

The goal of this redesign is to make the event type open and extensible: engine-defined events
and user-defined events co-exist in the same system with no changes to engine headers.

---

## Current Design (to be replaced)

```cpp
enum class EventType { None = 0, WindowClose, WindowResize, KeyPressed, ... };

#define EVENT_CLASS_TYPE(type)                                                  \
    static EventType get_static_type() { return EventType::type; }             \
    virtual EventType get_event_type() const override { return get_static_type(); } \
    virtual char const* get_name() const override { return #type; }

class Event
{
    virtual EventType get_event_type() const = 0;
    ...
};

// EventDispatcher matches on enum value
if (m_event.get_event_type() == TEvent::get_static_type())
    func(static_cast<TEvent&>(m_event));
```

Problems:
- `EventType` enum must be modified for every new event type.
- User-defined events cannot have a unique `EventType` value without patching engine code.
- Two classes can claim the same enum value (currently `TestEvent` used `None`), making
  `EventDispatcher` silently dispatch to the wrong type.

---

## Proposed Design

Use `std::type_index` as the event identity. Each concrete event class has a unique
`std::type_index` derived from its `typeid`, requiring no central registry.

### Event base class

Replace the enum-based virtual methods with a single `type_index` accessor:

```cpp
class Event
{
public:
    virtual ~Event() {}
    virtual std::type_index get_type_index() const = 0;
    virtual char const* get_name() const = 0;
    virtual std::string to_string() const { return get_name(); }
    bool get_handled() const { return m_handled; }
    void set_handled(bool handled) { m_handled = handled; }

private:
    bool m_handled{ false };
};
```

`EventType` enum is removed entirely.

### Macro replacement

```cpp
#define EVENT_CLASS_TYPE(TypeName)                                              \
    static std::type_index get_static_type()                                   \
    {                                                                           \
        return std::type_index(typeid(TypeName));                               \
    }                                                                           \
    std::type_index get_type_index() const override { return get_static_type(); } \
    char const* get_name() const override { return #TypeName; }
```

`get_static_type()` now returns a `std::type_index` unique to each class, with no enum
dependency.

### EventDispatcher

```cpp
template<typename TEvent>
requires(std::derived_from<TEvent, Event>)
bool dispatch(EventFn<TEvent> func)
{
    bool const is_matching_event = m_event.get_type_index() == TEvent::get_static_type();
    bool const is_event_handled  = m_event.get_handled();
    if (is_matching_event && !is_event_handled)
    {
        bool const now_handled = func(static_cast<TEvent&>(m_event));
        m_event.set_handled(now_handled);
        return true;
    }
    return false;
}
```

The only change from the current implementation is `get_event_type()` → `get_type_index()`
and `get_static_type()` returning `std::type_index`. The dispatch logic is otherwise identical.

---

## What Engine Events Look Like After Migration

Existing event classes only need the macro body to change — no other modifications:

```cpp
class KeyPressedEvent : public KeyEvent
{
public:
    KeyPressedEvent(SDL_Scancode scancode, bool is_repeat)
        : KeyEvent(scancode), m_is_repeat(is_repeat) {}

    // Before: EVENT_CLASS_TYPE(KeyPressed)
    // After:
    EVENT_CLASS_TYPE(KeyPressedEvent)   // pass the class name, not an enum member
    ...
};
```

The engine no longer needs the `EventType` enum in `event.h`, and `events_public.h` needs no
changes.

---

## What User-Defined Events Look Like

No engine changes required. A user defines an event anywhere in their game code:

```cpp
// game/events/player_events.h
#include "core/events/event.h"

class PlayerDiedEvent : public Core::Event
{
public:
    explicit PlayerDiedEvent(int player_id) : m_player_id(player_id) {}
    int get_player_id() const { return m_player_id; }

    EVENT_CLASS_TYPE(PlayerDiedEvent)

private:
    int m_player_id;
};
```

And dispatch it exactly like any engine event:

```cpp
void MyEntity::on_event(Core::Event& event)
{
    Core::EventDispatcher dispatcher(event);
    dispatcher.dispatch<PlayerDiedEvent>(
        [this](PlayerDiedEvent& e)
        {
            std::println("Player {} died", e.get_player_id());
            return false;
        }
    );
}
```

---

## Migration Checklist

1. Remove `EventType` enum from `event.h`.
2. Replace `get_event_type()` / `get_static_type()` virtual methods on `Event` with
   `get_type_index()` / `get_static_type()` returning `std::type_index`.
3. Update `EVENT_CLASS_TYPE` macro (pass class name instead of enum member name).
4. Update `EventDispatcher::dispatch<T>()` to compare `get_type_index()` against
   `TEvent::get_static_type()`.
5. Update all existing engine event classes to use the new macro signature.
6. Remove all `EventType::*` references (only used in the old macro and the old dispatcher).
7. Add `#include <typeindex>` to `event.h`.

---

## Considerations

- **Performance**: `std::type_index` comparison is a pointer comparison on most implementations
  (it wraps `std::type_info`). It is not slower than an integer enum comparison in practice.
- **Across DLL boundaries**: `std::type_index` / `typeid` identity can break when the same
  type is compiled into multiple translation units across DLL boundaries. If the engine is
  ever shipped as a DLL, a string-based key (e.g. hashed `type_info::name()`) may be needed
  instead. Not relevant for a statically-linked build.
- **`to_string()` / `get_name()`**: The `#TypeName` stringification in the macro gives the
  unqualified class name. For debugging this is usually sufficient, but a `namespace::ClassName`
  format can be obtained from `typeid(T).name()` (implementation-defined, often mangled on GCC/
  Clang; readable on MSVC).
