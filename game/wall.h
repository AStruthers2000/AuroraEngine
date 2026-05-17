////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PONG_WALL_H
#define PONG_WALL_H

#include "better_game_object.h"

class Wall : public BetterGameObject
{
public:
    Wall(AuroraEngine::GameWorld& owning_world, AuroraEngine::TransformComponent const& initial_transform, GameMode& owning_mode, glm::vec2 const& scale, SDL_Color const& color, float elasticity = 1.f)
        : BetterGameObject(owning_world, initial_transform, owning_mode, false)
        , m_color(color)
        , m_elasticity(elasticity)
    {
        get_transform().set_scale(scale);
    }

    ~Wall() override = default;

    void initialize() override;
    void update(float delta_time) override;
    void render(SDL_Renderer* renderer) override;
    void cleanup() override;
    float elasticity() const { return m_elasticity; }

private:
    SDL_Color m_color;
    float m_elasticity;
};


#endif //PONG_WALL_H
