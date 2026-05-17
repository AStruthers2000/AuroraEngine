////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PONG_BALL_H
#define PONG_BALL_H

#include <utility>

#include "better_game_object.h"
#include "player.h"

class Ball : public BetterGameObject, public std::enable_shared_from_this<Ball>
{
public:
    Ball(AuroraEngine::GameWorld& owning_world, AuroraEngine::TransformComponent const& initial_transform, GameMode& owning_mode, float radius, SDL_Color const& color)
        : BetterGameObject(owning_world, initial_transform, owning_mode, true)
        , m_spawn_transform(initial_transform)
        , m_color(color)
    {
        get_transform().set_scale(glm::vec2{radius * 2.f, radius * 2.f});
    }

    ~Ball() override = default;

    void initialize() override;
    void update(float delta_time) override;
    void render(SDL_Renderer* renderer) override;
    void cleanup() override;

private:
    SDL_Color m_color;

    AuroraEngine::TransformComponent m_spawn_transform;

    float m_min_speed = 100.f;
    float m_max_speed = 50000.f;

    void clamp_velocity();
    static glm::vec2 random_vector() ;
};


#endif //PONG_BALL_H
