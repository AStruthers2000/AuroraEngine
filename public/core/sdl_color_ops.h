////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Custom operations for SDL_Color and SDL_FColor.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_SDL_COLOR_OPS_H
#define CORE_SDL_COLOR_OPS_H

#include <SDL3/SDL.h>

inline bool operator==(SDL_Color lhs, SDL_Color rhs)
{
    bool const r_equal = lhs.r == rhs.r;
    bool const g_equal = lhs.g == rhs.g;
    bool const b_equal = lhs.b == rhs.b;
    bool const a_equal = lhs.a == rhs.a;
    return r_equal && g_equal && b_equal && a_equal;
}

inline bool operator!=(SDL_Color lhs, SDL_Color rhs)
{
    return !(lhs == rhs);
}

#endif // CORE_SDL_COLOR_OPS_H
