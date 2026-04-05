////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Input type definitions for the Aurora Engine.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ENGINE_INPUT_TYPES_H
#define ENGINE_INPUT_TYPES_H

#include "engine/time_keeper.h"

#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/constants.hpp>
#include <SDL3/SDL.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace AuroraEngine
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// KeyState types - shared state for tracking key press information
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class EKeyState
{
    Depressed,
    Pressed,
    Held,
    Released,
};

struct KeyState
{
    EKeyState key_state = EKeyState::Depressed;
    std::uint64_t pressed_frame = 0;
    std::uint64_t current_frame = 0;

    float get_held_duration() const
    {
        return time_delta<TIME_UNITS>(pressed_frame, current_frame);
    }

    std::string to_string() const
    {
        std::string msg = "KeyState{";
        std::string state_str;
        switch (key_state)
        {
            case EKeyState::Depressed:
                state_str = "Depressed";
                break;
            case EKeyState::Pressed:
                state_str = "Pressed";
                break;
            case EKeyState::Held:
                state_str = "Held";
                break;
            case EKeyState::Released:
                state_str = "Released";
                break;
            default:
                state_str = "Invalid";
                break;
        }
        msg += ".key_state = EKeyState::" + state_str +
             ", .pressed_frame = " + std::to_string(pressed_frame) +
             ", .current_frame = " + std::to_string(current_frame) +
             ", .held_duration = " + std::to_string(get_held_duration());
        msg += "}\n";
        return msg;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input processing types
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class EInputProcessedState
{
    ConsumeInput,
    DoNotConsumeInput,
};

struct InputActionArgs
{
    EInputProcessedState input_consume_state = EInputProcessedState::DoNotConsumeInput;
    std::uint8_t callback_priority = 100U;
    bool repeat_after_initial_trigger = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// InputAction - discrete key events (press, release, held for duration)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class InputAction
{
public:
    enum class ETriggerCondition
    {
        None,
        FireOnPress,
        FireOnRelease,
        FireAfterDuration,
    };

    InputAction(SDL_Scancode key_code, bool fire_when_key_is_pressed)
        : m_key_code(key_code)
        , m_fire_when_key_pressed(fire_when_key_is_pressed)
    {
        if (m_key_code != SDL_Scancode::SDL_SCANCODE_UNKNOWN)
        {
            m_trigger = m_fire_when_key_pressed ? ETriggerCondition::FireOnPress : ETriggerCondition::FireOnRelease;
        }
    }

    InputAction(SDL_Scancode key_code, float fire_after_duration)
        : m_key_code(key_code)
        , m_held_duration(fire_after_duration)
    {
        if (m_key_code != SDL_Scancode::SDL_SCANCODE_UNKNOWN)
        {
            m_trigger = ETriggerCondition::FireAfterDuration;
        }
    }

    ETriggerCondition get_trigger() const
    {
        return m_trigger;
    }

    SDL_Scancode get_key_code() const
    {
        return m_key_code;
    }

    float get_held_duration() const
    {
        return m_held_duration;
    }

    /// @brief Returns all scancodes that this action cares about.
    /// @return A vector containing the single key code bound to this action.
    std::vector<SDL_Scancode> get_bound_keys() const
    {
        if (m_key_code != SDL_Scancode::SDL_SCANCODE_UNKNOWN)
        {
            return {m_key_code};
        }
        return {};
    }

    /// @brief Determines if this action should trigger based on the current key state.
    /// @param key_state The current state of the key bound to this action.
    /// @param already_triggered Whether this action has already been triggered in the current press cycle.
    /// @param repeat_after_trigger Whether to allow repeated triggering after the initial trigger.
    /// @return True if the action should trigger, false otherwise.
    bool is_triggered(KeyState const& key_state, bool already_triggered, bool repeat_after_trigger) const
    {
        switch (m_trigger)
        {
            case ETriggerCondition::FireOnPress:
                return key_state.key_state == EKeyState::Pressed;

            case ETriggerCondition::FireOnRelease:
                return key_state.key_state == EKeyState::Released;

            case ETriggerCondition::FireAfterDuration:
                if (already_triggered && !repeat_after_trigger)
                {
                    return false;
                }
                return key_state.get_held_duration() >= m_held_duration;

            case ETriggerCondition::None:
            default:
                return false;
        }
    }

private:
    ETriggerCondition m_trigger = ETriggerCondition::None;
    SDL_Scancode m_key_code = SDL_Scancode::SDL_SCANCODE_UNKNOWN;
    bool m_fire_when_key_pressed = true;
    float m_held_duration = -1.f;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AxisState - processed axis state with utility functions for GameObjects
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Processed axis state containing raw trigger values and utility functions.
/// @details The raw glm::vec4 stores {x_pos, x_neg, y_pos, y_neg} trigger states.
///          Provides convenient accessors for normalized directions, angles, and magnitudes.
struct AxisState
{
    glm::vec4 raw{0.f, 0.f, 0.f, 0.f};

    /// @brief Returns the unnormalized direction vector.
    /// @return glm::vec2{x_pos - x_neg, y_pos - y_neg}
    [[nodiscard]] glm::vec2 get_direction_unnormalized() const
    {
        return {raw.x - raw.y, raw.z - raw.w};
    }

    /// @brief Returns the normalized direction vector in logical coordinates (+Y = up conceptually).
    /// @return Unit direction vector, or {0, 0} if no input.
    [[nodiscard]] glm::vec2 get_normalized() const
    {
        glm::vec2 dir = get_direction_unnormalized();
        float len = glm::length(dir);
        if (len > 0.f)
        {
            return glm::normalize(dir);
        }
        return {0.f, 0.f};
    }

    /// @brief Returns the normalized direction vector in screen coordinates (+Y = down).
    /// @return Unit direction vector with Y flipped for screen space, or {0, 0} if no input.
    [[nodiscard]] glm::vec2 get_normalized_screen_direction() const
    {
        glm::vec2 norm = get_normalized();
        return {norm.x, -norm.y};
    }

    /// @brief Returns the magnitude of the unnormalized direction.
    /// @return Length of the direction vector (0 to sqrt(2) for digital inputs).
    [[nodiscard]] float get_magnitude() const
    {
        return glm::length(get_direction_unnormalized());
    }

    /// @brief Returns the angle in radians where screen-up (-Y) is 0, increasing clockwise.
    /// @return Angle in radians [0, 2*pi), or 0 if no input.
    [[nodiscard]] float get_angle_radians() const
    {
        glm::vec2 dir = get_direction_unnormalized();
        if (dir.x == 0.f && dir.y == 0.f)
        {
            return 0.f;
        }
        // atan(x, y) gives angle from +Y axis, so we use atan2(x, y) to rotate reference, then normalize to [0, 2*pi)
        float angle = glm::atan(dir.x, dir.y);
        if (angle < 0.f)
        {
            angle += glm::two_pi<float>();
        }
        return angle;
    }

    /// @brief Returns the angle in degrees where screen-up (-Y) is 0°, increasing clockwise.
    /// @return Angle in degrees [0, 360), or 0 if no input.
    [[nodiscard]] float get_angle_degrees() const
    {
        return glm::degrees(get_angle_radians());
    }

    /// @brief Checks if there is any input on either axis.
    /// @return True if any trigger is active.
    [[nodiscard]] bool has_input() const
    {
        return raw.x != 0.f || raw.y != 0.f || raw.z != 0.f || raw.w != 0.f;
    }

    /// @brief Checks if there is any input on the X axis.
    /// @return True if x_pos or x_neg is active.
    [[nodiscard]] bool has_x_input() const
    {
        return raw.x != 0.f || raw.y != 0.f;
    }

    /// @brief Checks if there is any input on the Y axis.
    /// @return True if y_pos or y_neg is active.
    [[nodiscard]] bool has_y_input() const
    {
        return raw.z != 0.f || raw.w != 0.f;
    }

    /// @brief Returns the X axis value in [-1, 1].
    /// @return x_pos - x_neg
    [[nodiscard]] float get_x_axis() const
    {
        return raw.x - raw.y;
    }

    /// @brief Returns the Y axis value in [-1, 1].
    /// @return y_pos - y_neg
    [[nodiscard]] float get_y_axis() const
    {
        return raw.z - raw.w;
    }

    /// @brief Returns raw X axis trigger values.
    /// @return glm::vec2{x_pos, x_neg}
    [[nodiscard]] glm::vec2 get_x_axis_raw() const
    {
        return {raw.x, raw.y};
    }

    /// @brief Returns raw Y axis trigger values.
    /// @return glm::vec2{y_pos, y_neg}
    [[nodiscard]] glm::vec2 get_y_axis_raw() const
    {
        return {raw.z, raw.w};
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// InputAxis - continuous analog-like input from multiple keys
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class InputAxis
{
public:
    struct Axis
    {
        SDL_Scancode positive = SDL_Scancode::SDL_SCANCODE_UNKNOWN;
        SDL_Scancode negative = SDL_Scancode::SDL_SCANCODE_UNKNOWN;
    };

    explicit InputAxis(Axis const& x_axis, std::optional<Axis> y_axis = std::nullopt)
    {
        m_axes[0] = x_axis;
        m_axes[1] = y_axis.value_or(Axis{SDL_Scancode::SDL_SCANCODE_UNKNOWN, SDL_Scancode::SDL_SCANCODE_UNKNOWN});
    }

    bool update_axis(bool const* keyboard_state)
    {
        m_all_zeros_last_frame = m_axis_state == glm::vec4{0, 0, 0, 0};

        m_axis_state[0] = keyboard_state[m_axes[0].positive];
        m_axis_state[1] = keyboard_state[m_axes[0].negative];
        m_axis_state[2] = keyboard_state[m_axes[1].positive];
        m_axis_state[3] = keyboard_state[m_axes[1].negative];

        return m_axis_state != glm::vec4{0, 0, 0, 0} || !m_all_zeros_last_frame;
    }

    glm::vec4 const& get_internal_state() const { return m_axis_state; }

    /// @brief Returns all scancodes that this axis cares about.
    /// @return A vector containing all valid scancodes from both axes.
    std::vector<SDL_Scancode> get_bound_keys() const
    {
        std::vector<SDL_Scancode> keys;
        keys.reserve(4);

        if (m_axes[0].positive != SDL_Scancode::SDL_SCANCODE_UNKNOWN)
            keys.push_back(m_axes[0].positive);
        if (m_axes[0].negative != SDL_Scancode::SDL_SCANCODE_UNKNOWN)
            keys.push_back(m_axes[0].negative);
        if (m_axes[1].positive != SDL_Scancode::SDL_SCANCODE_UNKNOWN)
            keys.push_back(m_axes[1].positive);
        if (m_axes[1].negative != SDL_Scancode::SDL_SCANCODE_UNKNOWN)
            keys.push_back(m_axes[1].negative);

        return keys;
    }

private:
    Axis m_axes[2];
    glm::vec4 m_axis_state{};
    bool m_all_zeros_last_frame = true;
};

} // namespace AuroraEngine

#endif // ENGINE_INPUT_TYPES_H
