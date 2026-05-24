#include <format>
#include "game_mode.h"
#include "wall.h"
#include "ball.h"

void GameMode::initialize()
{
    Core::Engine::get().get_input_subsystem().register_callback(
        Core::InputAction(SDL_Scancode::SDL_SCANCODE_F12, true),
        [this]()
        {
            this->m_debug_mode = !this->m_debug_mode;
        });
}

void GameMode::update(float delta_time)
{
    float delta_to_last_spawn = Core::time_delta<TIME_UNITS>(m_last_scored_time);
    if (delta_to_last_spawn > SPAWN_BALL_WAIT_TIME)
    {
        spawn_ball(m_last_spawned_ball_specs);

        // Reset score time to prevent a new ball from spawning next frame. Realistically, the current time will never
        // reach uint64_t max - 5 seconds, so we're safe
        m_last_scored_time = std::numeric_limits<std::int64_t>::max();
    }

    // handle collision
    resolve_all_collision();

    std::erase_if(m_managed_objects, [](auto managed_object)
    {
       return managed_object.expired();
    });
}

void GameMode::render(SDL_Renderer *renderer)
{
    SDL_FRect logical_resolution;
    SDL_GetRenderLogicalPresentationRect(renderer, &logical_resolution);

    std::string p1_text = std::format("Player 1: {:>3}", m_player_1_points);
    std::string p2_text = std::format("Player 2: {:>3}", m_player_2_points);

    // Debug render text is 8 x 8 pixels
    std::size_t const char_size{ 8 };

    float p1_text_x{ 35.f }; // 25 for wall thickness and 10 for padding
    float text_y{ (25.f / 2.f) - (char_size / 2.f) + 0.5f};

    std::size_t str_len = p2_text.length() * char_size;
    float p2_text_x{ logical_resolution.w - p1_text_x - static_cast<float>(str_len) };

    SDL_SetRenderDrawColor(renderer, 215, 201, 170, 255);
    SDL_RenderDebugText(renderer, p1_text_x, text_y, p1_text.c_str());
    SDL_RenderDebugText(renderer, p2_text_x, text_y, p2_text.c_str());

    if (m_debug_mode)
    {
        for (auto const &managed_object: m_managed_objects)
        {
            if (auto object = managed_object.lock())
            {
                SDL_FRect collider = object->get_collider();
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderRect(renderer, &collider);
            }
        }
    }
}

void GameMode::cleanup()
{

}

void GameMode::spawn_player(PlayerSpecification const& player_spec)
{
    std::shared_ptr<Player> player = std::make_unique<Player>(get_world(), player_spec.position, *this, player_spec.color, player_spec.player_num);
    player->get_transform().set_scale(player_spec.size);
    m_managed_objects.push_back(player);
    get_world().add_object(std::move(player), player_spec.update_order);
}

void GameMode::spawn_ball(BallSpecification const& ball_spec)
{
    Core::TransformComponent spawn_location{ ball_spec.position };
    spawn_location.update_position(glm::vec2(-ball_spec.radius, -ball_spec.radius));

    std::shared_ptr<Ball> ball = std::make_unique<Ball>(get_world(), spawn_location, *this, ball_spec.radius, ball_spec.color);
    m_managed_objects.push_back(ball);
    get_world().add_object(std::move(ball), ball_spec.update_order);

    m_last_spawned_ball_specs = ball_spec;
}

void GameMode::spawn_wall(const WallSpecification &wall_spec)
{
    std::shared_ptr<Wall> wall = std::make_shared<Wall>(get_world(), Core::TransformComponent(wall_spec.position), *this, wall_spec.scale, wall_spec.color, wall_spec.elasticity);
    m_managed_objects.push_back(wall);
    get_world().add_object(std::move(wall), wall_spec.update_order);
}

void GameMode::spawn_overlap(const OverlapSpecification &overlap_spec)
{
    std::shared_ptr<OverlapVolume> overlap = std::make_unique<OverlapVolume>(get_world(), Core::TransformComponent(overlap_spec.position), *this, overlap_spec.overlap_position);
    overlap->get_transform().set_scale(overlap_spec.scale);
    m_overlap_volumes.push_back(overlap);
    get_world().add_object(std::move(overlap), overlap_spec.update_order);
}

void GameMode::resolve_all_collision()
{
    // Cache all colliders at the beginning
    std::unordered_map<BetterGameObject*, SDL_FRect> colliders;
    for (auto const& object : m_managed_objects)
    {
        if (auto object_ptr = object.lock())
        {
            if (object_ptr->get_object_state() == EGameObjectState::Active)
            {
                colliders.emplace(object_ptr.get(), object_ptr->get_collider());
            }
        }
    }

    // iterate over all dynamic objects
    for (auto& [object, collider] : colliders)
    {
        // for each object that isn't this object
        if (object->is_dynamic())
        {
            for (auto& [other, other_collider] : colliders)
            {
                if (object != other)
                {
                    SDL_FRect overlap;
                    if (SDL_GetRectIntersectionFloat(&collider, &other_collider, &overlap))
                    {
                        constexpr float tolerance = 0.001f;
                        if (abs(overlap.w) > tolerance && abs(overlap.h) > tolerance)
                        {
                            resolve_collision(object, other, overlap);
                        }
                    }
                }
            }
        }
    }
}

void GameMode::resolve_collision(BetterGameObject* dynamic, BetterGameObject* other, SDL_FRect const& overlap)
{
    Core::TransformComponent dynamic_transform = dynamic->get_transform();
    glm::vec2 dynamic_pos = dynamic->get_transform().get_position();
    glm::vec2 dynamic_size = dynamic->get_transform().get_scale();
    glm::vec2 dynamic_center = dynamic_pos + dynamic_size * 0.5f;
    glm::vec2 overlap_center = glm::vec2{overlap.x + overlap.w * 0.5f,
                                         overlap.y + overlap.h * 0.5f};

    ECollisionDirection direction{ ECollisionDirection::None };
    glm::vec2 collision_vec = dynamic_center - overlap_center;
    if (abs(collision_vec.x) <= abs(collision_vec.y))
    {
        // Collision happened further away vertically, so this is a vertical collision
        if (collision_vec.y > 0)
        {
            // Collision happened above our center
            dynamic->get_transform().update_position(glm::vec2(0, overlap.h));
            direction = ECollisionDirection::Up;
        }
        else
        {
            // Collision happened below our center
            dynamic->get_transform().update_position(glm::vec2(0, -overlap.h));
            direction = ECollisionDirection::Down;
        }
    }
    else
    {
        // Collision happened further away horizontally, so this is a horizontal collision
        if (collision_vec.x > 0)
        {
            // Collision happened to the left of our center
            dynamic->get_transform().update_position(glm::vec2(overlap.w, 0));
            direction = ECollisionDirection::Left;
        }
        else
        {
            // Collision happened to the right of our center
            dynamic->get_transform().update_position(glm::vec2(-overlap.w, 0));
            direction = ECollisionDirection::Right;
        }
    }
    dynamic->get_transform().set_velocity(glm::vec2(0));
    // Call dynamic object's collision response function with transform before hit, other object, and direction of hit
    dynamic->collision_response(dynamic_transform, other, direction);
}

void GameMode::score_goal(int player_that_scored)
{
    bool player_scored{ true };
    if (player_that_scored == 1)
    {
        m_player_1_points++;
    }
    else if (player_that_scored == 2)
    {
        m_player_2_points++;
    }
    else
    {
        printf("Player %d doesn't exist\n", player_that_scored);
        player_scored = false;
    }

    if (player_scored)
    {
        m_last_scored_time = Core::get_current_time();
    }
}
