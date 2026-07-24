////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_COMPONENTS_TRANSFORM_COMPONENT_H
#define CORE_COMPONENTS_TRANSFORM_COMPONENT_H

#include "core/component.h"

#include <glm/glm.hpp>

namespace Core
{

class TransformComponent : public Component
{
public:
    static constexpr bool unique_per_entity = true;

    explicit TransformComponent(Entity& owning_entity)
      : Component(owning_entity)
    {
    }
    TransformComponent(Entity& owning_entity, const glm::vec2& position)
      : Component(owning_entity)
      , m_local_position(position)
    {
    }
    TransformComponent(Entity& owning_entity,
                       const glm::vec2& position,
                       const glm::vec2& scale)
      : Component(owning_entity)
      , m_local_position(position)
      , m_local_scale(scale)
    {
    }
    // TransformComponent(Entity& owning_entity,
    //                    const glm::vec2& position,
    //                    const glm::vec2& velocity = glm::vec2(0.0f, 0.0f),
    //                    const glm::vec2& acceleration = glm::vec2(0.0f, 0.0f),
    //                 //    float rotation = 0.0f,
    //                    const glm::vec2& scale = glm::vec2(1.0f, 1.0f))
    //     : Component(owning_entity)
    //     , m_local_position(position)
    //     , m_velocity(velocity)
    //     , m_acceleration(acceleration)
    //     //   m_rotation(rotation),
    //     ,  m_local_scale(scale)
    // {
    // }

    ~TransformComponent() override = default;

    const glm::vec2& get_local_position() const { return m_local_position; }
    void set_local_position(const glm::vec2& position) { m_local_position = position; }
    void update_local_position(const glm::vec2& delta) { m_local_position += delta; }

    /// @brief Returns the position in world space, accumulated from the parent chain.
    glm::vec2 get_world_position() const;

    /// @brief Returns the scale in world space, accumulated from the parent chain.
    glm::vec2 get_world_scale() const;

    const glm::vec2& get_velocity() const { return m_velocity; }
    void set_velocity(const glm::vec2& velocity) { m_velocity = velocity; }
    void update_velocity(const glm::vec2& delta) { m_velocity += delta; }

    const glm::vec2& get_acceleration() const { return m_acceleration; }
    void set_acceleration(const glm::vec2& acceleration) { m_acceleration = acceleration; }
    void update_acceleration(const glm::vec2& delta) { m_acceleration += delta; }

    // float get_rotation() const { return m_rotation; }
    // void set_rotation(float rotation) { m_rotation = rotation; }
    // void update_rotation(float delta) { m_rotation += delta; }

    const glm::vec2& get_local_scale() const { return m_local_scale; }
    void set_local_scale(const glm::vec2& scale) { m_local_scale = scale; }
    void update_local_scale(const glm::vec2& delta) { m_local_scale += delta; }

private:
    glm::vec2 m_local_position{0.0f, 0.0f};
    glm::vec2 m_velocity{0.0f, 0.0f};
    glm::vec2 m_acceleration{0.0f, 0.0f};
    // float m_rotation = 0.0f;
    glm::vec2 m_local_scale{1.0f, 1.0f};
};

} // namespace Core

#endif // CORE_COMPONENTS_TRANSFORM_COMPONENT_H
