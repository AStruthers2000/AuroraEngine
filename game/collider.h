////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PONG_COLLIDER_H
#define PONG_COLLIDER_H

#include "aurora_engine_public.h"

#include <functional>

enum class ECollisionDirection
{
    None,
    Up,
    Down,
    Left,
    Right
};

class BetterGameObject;

using CollisionCallback = std::function<void(AuroraEngine::TransformComponent const&, BetterGameObject*, ECollisionDirection)>;

class Collider
{
public:
    enum class ECollisionType
    {
        Block,
    };

    Collider()
        : m_owner(nullptr)
    {
    }

    Collider(AuroraEngine::GameObject* owner, bool is_dynamic)
        : m_owner(owner)
        , m_is_dynamic(is_dynamic)
    {
    }

    SDL_FRect get_collider() const
    {
        SDL_FRect extents{};
        if (m_owner)
        {
            extents = {
                    .x = m_owner->get_transform().get_position().x,
                    .y = m_owner->get_transform().get_position().y,
                    .w = m_owner->get_transform().get_scale().x,
                    .h = m_owner->get_transform().get_scale().y,
            };
        }

        return extents;
    }

    AuroraEngine::GameObject* get_owner() const { return m_owner; }

    bool is_dynamic() const { return m_is_dynamic; }

    void register_collision_response(CollisionCallback&& collision_response)
    {
        m_custom_collision_response = std::move(collision_response);
    }

    void collision_response(AuroraEngine::TransformComponent const& my_transform, BetterGameObject* other_object, ECollisionDirection direction)
    {
        if (m_custom_collision_response)
        {
            m_custom_collision_response(my_transform, other_object, direction);
        }
    }

private:
    AuroraEngine::GameObject* m_owner;
    bool m_is_dynamic{ false };
    CollisionCallback m_custom_collision_response{ nullptr };
};

#endif //PONG_COLLIDER_H
