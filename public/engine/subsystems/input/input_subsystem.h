////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ENGINE_SUBSYSTEMS_INPUT_SUBSYSTEM_H
#define ENGINE_SUBSYSTEMS_INPUT_SUBSYSTEM_H

#include "input_types.h"

#include <algorithm>
#include <functional>
#include <set>
#include <unordered_map>
#include <variant>
#include <vector>

namespace AuroraEngine
{

using InputCallback_Action = std::function<void()>;
using InputCallback_Axis = std::function<void(AxisState const&)>;

constexpr std::uint8_t DEFAULT_CALLBACK_PRIORITY = 100U;

class InputSubsystem
{
public:
    InputSubsystem() = default;
    ~InputSubsystem() = default;

    void register_callback(InputAction const& input_action, InputCallback_Action callback, std::optional<InputActionArgs> args = std::nullopt);
    void register_callback(InputAxis const& input_axis, InputCallback_Axis callback);

    bool test();

private:
    /// @brief Unified callback structure that holds either an InputAction or InputAxis binding
    ///        along with its associated callback. This replaces the separate InputActionCallback
    ///        and InputAxisCallback structs.
    struct InputBindingCallback
    {
        std::variant<InputAction, InputAxis> binding;
        std::variant<InputCallback_Action, InputCallback_Axis> callback;
        std::optional<InputActionArgs> args;  // Only populated for InputAction bindings
        bool triggered = false;               // Only meaningful for InputAction with FireAfterDuration

        /// @brief Helper to get all bound keys from the binding variant
        std::vector<SDL_Scancode> get_bound_keys() const
        {
            return std::visit([](auto const& b) { return b.get_bound_keys(); }, binding);
        }

        /// @brief Check if this binding is an InputAction
        bool is_action() const
        {
            return std::holds_alternative<InputAction>(binding);
        }

        /// @brief Check if this binding is an InputAxis
        bool is_axis() const
        {
            return std::holds_alternative<InputAxis>(binding);
        }
    };

    void update_cached_keyboard_map();
    void update_keyboard_state(bool const* states, std::uint64_t current_frame);
    void process_bindings(bool const* states);

    std::set<SDL_Scancode> m_keys_we_care_about{};
    std::unordered_map<SDL_Scancode, KeyState> m_keyboard_state{};
    std::vector<InputBindingCallback> m_bindings{};
    bool m_recalculate_cache = false;
};

} // namespace AuroraEngine

#endif //ENGINE_SUBSYSTEMS_INPUT_SUBSYSTEM_H
