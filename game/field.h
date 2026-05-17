////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PONG_FIELD_H
#define PONG_FIELD_H

#include "aurora_engine_public.h"

#include <string>

class Field : public AuroraEngine::GameObject
{
public:
    Field(AuroraEngine::GameWorld& owning_world,
          AuroraEngine::TransformComponent const& initial_transform,
          glm::vec2 const& size,
          std::string texture_path)
        : GameObject(owning_world, initial_transform)
        , m_size(size)
        , m_texture_path(std::move(texture_path))
    {
    }

    ~Field() override;

    void initialize() override;
    void update(float delta_time) override;
    void render(SDL_Renderer* renderer) override;
    void cleanup() override;

private:
    glm::vec2 m_size;
    std::string m_texture_path;
    SDL_Texture* m_texture = nullptr;
};

#endif //PONG_FIELD_H
