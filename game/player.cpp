#include "player.h"
#include "ball.h"
#include <cmath>

void Player::initialize()
{
    SDL_Scancode up = m_player_num == 1 ? SDL_Scancode::SDL_SCANCODE_W : SDL_Scancode::SDL_SCANCODE_UP;
    SDL_Scancode down = m_player_num == 1 ? SDL_Scancode::SDL_SCANCODE_S : SDL_Scancode::SDL_SCANCODE_DOWN;
    AuroraEngine::Engine::get().get_input_subsystem().register_callback(
            AuroraEngine::InputAxis(AuroraEngine::InputAxis::Axis{SDL_Scancode::SDL_SCANCODE_UNKNOWN,
                                                                  SDL_Scancode::SDL_SCANCODE_UNKNOWN},
                                    AuroraEngine::InputAxis::Axis{up, down}),
            [this](AuroraEngine::AxisState const &axis) {
                get_transform().set_acceleration(axis.get_normalized_screen_direction() * m_acceleration_speed);
            });

    register_collision_response([this](AuroraEngine::TransformComponent const& my_transform, BetterGameObject* other_object, ECollisionDirection direction)
    {
        if (dynamic_cast<Ball*>(other_object))
        {
            get_transform().set_position(my_transform.get_position());
            get_transform().set_velocity(my_transform.get_velocity());
            get_transform().set_acceleration(my_transform.get_acceleration());
        }
    });
}

void Player::update(float delta_time)
{
    auto clamp_magnitude = [](glm::vec2 const &v, float max) -> glm::vec2 {
        float len = glm::length(v);
        if (len > max && len > 0.f)
            return (v / len) * max;
        return v;
    };

    get_transform().set_acceleration(clamp_magnitude(get_transform().get_acceleration(), m_max_acceleration));
    get_transform().update_velocity(get_transform().get_acceleration() * delta_time);
    get_transform().set_velocity(
            clamp_magnitude(get_transform().get_velocity() * std::pow(m_drag, delta_time), m_max_speed));
    get_transform().update_position(get_transform().get_velocity() * delta_time);
}

void Player::render(SDL_Renderer* renderer)
{
    SDL_FRect dst{
        .x = get_transform().get_position().x,
        .y = get_transform().get_position().y,
        .w = get_transform().get_scale().x,
        .h = get_transform().get_scale().y,
    };

    SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
    SDL_RenderFillRect(renderer, &dst);
}

void Player::cleanup()
{

}