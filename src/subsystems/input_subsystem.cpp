#include "engine/subsystems/input/input_subsystem.h"
#include "engine/engine.h"

namespace AuroraEngine
{

//--------------------------------------------------------------------------------------------------
void InputSubsystem::register_callback(InputAction const& input_action, InputCallback_Action callback, std::optional<InputActionArgs> args)
{
    // Construct default args if args aren't present
    InputActionArgs action_args = args.value_or(
        InputActionArgs{
            .input_consume_state = EInputProcessedState::DoNotConsumeInput,
            .callback_priority = DEFAULT_CALLBACK_PRIORITY,
            .repeat_after_initial_trigger = false,
        }
    );

    // Register all keys this action cares about
    for (auto const& key : input_action.get_bound_keys())
    {
        auto [it, inserted] = m_keys_we_care_about.emplace(key);
        if (inserted)
        {
            m_recalculate_cache = true;
        }
    }

    // Add unified binding
    m_bindings.push_back({
        .binding = input_action,
        .callback = std::move(callback),
        .args = action_args,
        .triggered = false,
    });
}

//--------------------------------------------------------------------------------------------------
void InputSubsystem::register_callback(InputAxis const& input_axis, InputCallback_Axis callback)
{
    // Register all keys this axis cares about
    for (auto const& key : input_axis.get_bound_keys())
    {
        auto [it, inserted] = m_keys_we_care_about.emplace(key);
        if (inserted)
        {
            m_recalculate_cache = true;
        }
    }

    // Add unified binding
    m_bindings.push_back({
        .binding = input_axis,
        .callback = std::move(callback),
        .args = std::nullopt,
        .triggered = false,
    });
}

//--------------------------------------------------------------------------------------------------
void InputSubsystem::update_cached_keyboard_map()
{
    for (auto const& key_code : m_keys_we_care_about)
    {
        // If the key code isn't already in the map
        if (!m_keyboard_state.contains(key_code))
        {
            // Add state to the map
            m_keyboard_state[key_code] = KeyState{};
        }
    }

    // Eventually we will support unregistering callbacks, so we will need a part of the cache
    // function to remove states from the keyboard map
}

//--------------------------------------------------------------------------------------------------
void InputSubsystem::update_keyboard_state(bool const* states, std::uint64_t current_frame)
{
    if (m_recalculate_cache)
    {
        update_cached_keyboard_map();
        m_recalculate_cache = false;
    }

    for (auto const &key_code: m_keys_we_care_about)
    {
        // Get the state of this key. Update this key's state in the keyboard map as follows:
        // * If states[key_code] == true, m_keyboard_state[key_code].key_state transitions with:
        //      - Depressed -> Pressed, pressed_frame = current_frame
        //      - Pressed -> Held
        //      - Held -> current_frame = current_frame
        // * If states[key_code] == false, m_keyboard_state[key_code].key_state transitions with:
        //      - Pressed, Held -> Released
        //      - Released -> Depressed, pressed_frame = 0, current_frame = 0

        KeyState &key_state = m_keyboard_state[key_code];
        if (states[key_code])
        {
            switch (key_state.key_state)
            {
                case EKeyState::Depressed:
                    key_state.key_state = EKeyState::Pressed;
                    key_state.pressed_frame = current_frame;
                    key_state.current_frame = current_frame;
                    break;
                case EKeyState::Pressed:
                    key_state.key_state = EKeyState::Held;
                    key_state.current_frame = current_frame;
                    break;
                case EKeyState::Held:
                    key_state.current_frame = current_frame;
                    break;
                case EKeyState::Released:
                    [[fallthrough]];
                default:
                    break;
            }
        }
        else
        {
            switch (key_state.key_state)
            {
                case EKeyState::Pressed:
                    [[fallthrough]];
                case EKeyState::Held:
                    key_state.key_state = EKeyState::Released;
                    key_state.current_frame = current_frame;
                    break;
                case EKeyState::Released:
                    // Reset key state and triggered flags for all bindings on this key
                    key_state = KeyState{};
                    for (auto &binding_callback: m_bindings)
                    {
                        if (binding_callback.is_action())
                        {
                            auto const &action = std::get<InputAction>(binding_callback.binding);
                            if (action.get_key_code() == key_code)
                            {
                                binding_callback.triggered = false;
                            }
                        }
                    }
                    break;
                case EKeyState::Depressed:
                    [[fallthrough]];
                default:
                    break;
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
void InputSubsystem::process_bindings(bool const* states)
{
    for (auto &binding_callback: m_bindings)
    {
        std::visit([&](auto &binding) {
            using T = std::decay_t<decltype(binding)>;

            if constexpr (std::is_same_v<T, InputAction>)
            {
                // Process InputAction
                SDL_Scancode key = binding.get_key_code();
                KeyState const &key_state = m_keyboard_state[key];

                bool repeat = binding_callback.args.has_value() && binding_callback.args->repeat_after_initial_trigger;

                if (binding.is_triggered(key_state, binding_callback.triggered, repeat))
                {
                    binding_callback.triggered = true;
                    std::get<InputCallback_Action>(binding_callback.callback)();
                }
            }
            else if constexpr (std::is_same_v<T, InputAxis>)
            {
                // Process InputAxis - uses raw bool states directly
                if (binding.update_axis(states))
                {
                    AxisState axis_state{binding.get_internal_state()};
                    std::get<InputCallback_Axis>(binding_callback.callback)(axis_state);
                }
            }
        }, binding_callback.binding);
    }
}

//--------------------------------------------------------------------------------------------------
bool InputSubsystem::test()
{
    SDL_Event event{ 0 };
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EventType::SDL_EVENT_QUIT:
                return false;
            case SDL_EventType::SDL_EVENT_WINDOW_RESIZED:
                Engine::get().get_window().set_size({event.window.data1, event.window.data2});
                break;
        }
    }

    bool const* states = SDL_GetKeyboardState(nullptr);
    std::uint64_t current_frame = get_current_time();

    update_keyboard_state(states, current_frame);
    process_bindings(states);

    return true;
}

} // namespace AuroraEngine
