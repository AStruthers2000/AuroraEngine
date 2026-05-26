////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PONG_WALL_H
#define PONG_WALL_H

// #include "better_game_object.h"
#include "aurora_engine_public.h"

class Wall : public Core::Entity
{
public:
    Wall(Core::Layer& owner, std::uint8_t update_order, glm::vec2 position, glm::vec2 scale);
    // Wall(Core::GameWorld& owning_world, Core::TransformComponent const& initial_transform, GameMode& owning_mode, glm::vec2 const& scale, SDL_Color const& color, float elasticity = 1.f)
    //     : BetterGameObject(owning_world, initial_transform, owning_mode, false)
    //     , m_color(color)
    //     , m_elasticity(elasticity)
    // {
    //     get_transform().set_scale(scale);
    // }

    ~Wall() override = default;

    void awake_entity() override;
    void render_entity(SDL_Renderer* renderer) override;
    // float elasticity() const { return m_elasticity; }

private:
    // SDL_Color m_color;
    // float m_elasticity;
    Core::TransformComponent* transform{ nullptr };
    glm::vec2 m_position;
    glm::vec2 m_size;
};


#endif //PONG_WALL_H
