#include "engine/game_object.h"

namespace AuroraEngine
{

//--------------------------------------------------------------------------------------------------
GameObject::GameObject(GameWorld& owning_world)
    : m_world(owning_world),
      m_transform({0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, 0.f, {1.f, 1.f})
{
}

//--------------------------------------------------------------------------------------------------
GameObject::GameObject(GameWorld& owning_world, TransformComponent const& initial_transform)
    : m_world(owning_world),
      m_transform(initial_transform)
{
}

} // namespace AuroraEngine
