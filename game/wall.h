////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PONG_WALL_H
#define PONG_WALL_H

#include "aurora_engine_public.h"

class Wall : public Core::Entity
{
public:
    Wall(Core::Layer& owner, std::uint8_t update_order, glm::vec2 position, glm::vec2 scale);
    ~Wall() override = default;

private:
    Core::Component::Order m_component_order
    {
        .update_order = Core::DEFAULT_SORTING_ORDER,
        .render_order = Core::DEFAULT_SORTING_ORDER,
    };

    SDL_Color m_color
    {
        .r = 25,
        .g = 83,
        .b = 95,
        .a = SDL_ALPHA_OPAQUE
    };
};


#endif //PONG_WALL_H
