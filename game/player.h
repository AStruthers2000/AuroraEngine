////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

//#include "engine/game_object.h"
#include "aurora_engine_public.h"

namespace TestGame
{

class Player : public AuroraEngine::GameObject
{
public:
    Player(AuroraEngine::GameWorld& owning_world) : GameObject(owning_world)
    {}

    Player(AuroraEngine::GameWorld& owning_world, AuroraEngine::TransformComponent const& transform) 
        : GameObject(owning_world, transform)
    {}

    ~Player()
    {
        cleanup();
    }

    void initialize() override;
    void update(float delta_time) override;
    void render(SDL_Renderer* renderer) override;
    void cleanup() override;

private:
    SDL_Texture* m_texture;

    int direction = 0;
    float speed = 100.f;
    glm::vec2 move_vec{0, 0};
};

} // namespace TestGame

#endif // GAME_PLAYER_H
