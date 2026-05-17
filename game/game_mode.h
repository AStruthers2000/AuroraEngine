////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PONG_GAME_MODE_H
#define PONG_GAME_MODE_H

#include "aurora_engine_public.h"
#include "better_game_object.h"
#include "overlap_volume.h"

struct PlayerSpecification
{
    glm::vec2 position;
    glm::vec2 size;
    SDL_Color color;
    int player_num;
    int update_order;
};
struct BallSpecification
{
    glm::vec2 position;
    float radius;
    SDL_Color color;
    int update_order;
};

struct WallSpecification
{
    glm::vec2 position;
    glm::vec2 scale;
    SDL_Color color;
    float elasticity;
    int update_order;
};

struct OverlapSpecification
{
    glm::vec2 position;
    glm::vec2 scale;
    OverlapVolume::EOverlapPosition overlap_position;
    int update_order;
};

class GameMode : public AuroraEngine::GameObject
{
public:
    GameMode(AuroraEngine::GameWorld& owning_world, AuroraEngine::TransformComponent const& initial_transform)
    : GameObject(owning_world, initial_transform)
    {
    }

    ~GameMode() override = default;

    void initialize() override;
    void update(float delta_time) override;
    void render(SDL_Renderer* renderer) override;
    void cleanup() override;

    void spawn_player(PlayerSpecification const& player_spec);
    void spawn_ball(BallSpecification const& ball_spec);
    void spawn_wall(WallSpecification const& wall_spec);
    void spawn_overlap(OverlapSpecification const& overlap_spec);
    std::vector<std::weak_ptr<OverlapVolume>>& get_overlap_volumes() { return m_overlap_volumes; }

    void score_goal(int player_that_scored);

private:
    void resolve_all_collision();
    void resolve_collision(BetterGameObject* dynamic, BetterGameObject* other, SDL_FRect const& overlap);

    std::vector<std::weak_ptr<BetterGameObject>> m_managed_objects;
    std::vector<std::weak_ptr<OverlapVolume>> m_overlap_volumes;

    int m_player_1_points{ 0 };
    int m_player_2_points{ 0 };
    BallSpecification m_last_spawned_ball_specs;
    std::uint64_t m_last_scored_time{ std::numeric_limits<std::int64_t>::max() };
    static constexpr float SPAWN_BALL_WAIT_TIME{ 1.75f };

    bool m_debug_mode{ false };
};
#endif //PONG_GAME_MODE_H
