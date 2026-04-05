////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ENGINE_TIME_KEEPER_H
#define ENGINE_TIME_KEEPER_H

#include <SDL3/SDL.h>

#include <cmath>
#include <cstdint>

namespace
{

enum class ETimeType
{
    Nanoseconds,
    Milliseconds,
};

constexpr ETimeType TIME_UNITS  = ETimeType::Nanoseconds;
constexpr float NANOSEC_TO_SEC  = 1E+09;
constexpr float MILLISEC_TO_SEC = 1E+03;
constexpr float SEC_FACTOR      = TIME_UNITS == ETimeType::Nanoseconds ? NANOSEC_TO_SEC : MILLISEC_TO_SEC;

} // anonymous namespace

namespace AuroraEngine
{

/// @brief Returns the current time in the units specified by the TIME_UNITS constant.
/// @return The current time in the specified units.
static std::uint64_t get_current_time()
{
    std::uint64_t cur_time = 0;
    switch (TIME_UNITS)
    {
        case ETimeType::Milliseconds:
            cur_time = SDL_GetTicks();
            break;
        case ETimeType::Nanoseconds:
            cur_time = SDL_GetTicksNS();
            break;
        default:
            break;
    }
    return cur_time;
}

/// @brief Returns the current time in seconds, based on the time unit specified by the TIME_UNITS constant.
/// @return The current time in seconds.
static float time_to_seconds()
{
    return static_cast<float>(get_current_time()) / SEC_FACTOR;
}

/// @brief Calculates a delta between compare_time and current_time, returning the delta in seconds. If keep_sign is false, the absolute value of the delta is returned.
/// @tparam T The unit of time to use for the delta calculation. Must be a value in the ETimeType enum.
/// @param compare_time The time to compare against the current time, in the same units as specified by the template parameter T.
/// @param keep_sign If true, the delta retains its sign; if false, the absolute value of the delta is returned. Defaults to true.
/// @return The delta between compare_time and the current time, in seconds.
template <ETimeType T>
static float time_delta(std::uint64_t const compare_time, bool const keep_sign = true)
{
    constexpr float factor = T == ETimeType::Nanoseconds ? NANOSEC_TO_SEC : MILLISEC_TO_SEC;
    std::uint64_t current_time = get_current_time();
    std::int64_t delta = static_cast<std::int64_t>(current_time) - static_cast<std::int64_t>(compare_time);
    float delta_seconds = static_cast<float>(delta) / factor;
    return keep_sign ? delta_seconds : std::abs(delta_seconds);
}

/// @brief Calculates a delta between start_time and end_time, returning the delta in seconds. If keep_sign is false, the absolute value of the delta is returned.
/// @tparam T The unit of time to use for the delta calculation. Must be a value in the ETimeType enum.
/// @param start_time The start time, in the same units as specified by the template parameter T.
/// @param end_time The end time, in the same units as specified by the template parameter T.
/// @param keep_sign If true, the delta retains its sign; if false, the absolute value of the delta is returned. Defaults to true.
/// @return The delta between start_time and end_time, in seconds.
template<ETimeType T>
static float time_delta(std::uint64_t const start_time, std::uint64_t const end_time, bool const keep_sign = true)
{
    constexpr float factor = T == ETimeType::Nanoseconds ? NANOSEC_TO_SEC : MILLISEC_TO_SEC;
    std::int64_t delta = static_cast<std::int64_t>(end_time) - static_cast<std::int64_t>(start_time);
    float delta_seconds = static_cast<float>(delta) / factor;
    return keep_sign ? delta_seconds : std::abs(delta_seconds);
}

} // namespace AuroraEngine

#endif // ENGINE_TIME_KEEPER_H
