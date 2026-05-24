#include "aurora_engine_public.h"

#include "field.h"
#include "game_mode.h"

#include <array>
#include <random>

using namespace Core;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Window info
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr WindowSpecification window_spec
{
    .window_size = {1628, 1074},
    .logical_size = {1628, 1074},
    .title = "Pong",
};

constexpr glm::vec2 mid_screen = window_spec.window_size / 2.f;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Wall info
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr SDL_Color wall_color{25, 83, 95, 255};
constexpr float wall_thickness = 25.f;
constexpr float wall_elasticity = 0.9f;
constexpr int wall_update_order = 0;

std::vector<WallSpecification> wall_specs
{
    {
        .position = {0.f, 0.f},
        .scale = {window_spec.window_size.x, wall_thickness},
        .color = wall_color,
        .elasticity = wall_elasticity,
        .update_order = wall_update_order,
    }, // top wall
    {
        .position = {0.f, window_spec.window_size.y - wall_thickness},
        .scale = {window_spec.window_size.x, wall_thickness},
        .color = wall_color,
        .elasticity = wall_elasticity,
        .update_order = wall_update_order,
    }, // bottom wall
    {
        .position = {0.f, 0.f},
        .scale = {wall_thickness, 235.f},
        .color = wall_color,
        .elasticity = wall_elasticity,
        .update_order = wall_update_order,
    }, // top-left wall
    {
        .position = {0.f, window_spec.window_size.y - 234.f},
        .scale = {wall_thickness, 234.f},
        .color = wall_color,
        .elasticity = wall_elasticity,
        .update_order = wall_update_order,
    }, // bottom-left wall
    {
        .position = {window_spec.window_size.x - wall_thickness, 0.f},
        .scale = {wall_thickness, 235.f},
        .color = wall_color,
        .elasticity = wall_elasticity,
        .update_order = wall_update_order,
    }, // top-right wall
    {
        .position = {window_spec.window_size.x - wall_thickness, window_spec.window_size.y - 234.f},
        .scale = {wall_thickness, 234.f},
        .color = wall_color,
        .elasticity = wall_elasticity,
        .update_order = wall_update_order,
    }, // bottom-right wall
};


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Overlap info
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr float overlap_extra = 100.f;
constexpr int overlap_update_order = -1;

std::vector<OverlapSpecification> overlap_specs
{
    {
        .position = {0.f - overlap_extra, 234.f - overlap_extra},
        .scale = {overlap_extra, 607.f + (overlap_extra * 2)},
        .overlap_position = OverlapVolume::EOverlapPosition::LeftSide,
        .update_order = overlap_update_order,
    },
    {
        .position = {window_spec.window_size.x, 234.f - overlap_extra},
        .scale = {overlap_extra, 607.f + (overlap_extra * 2)},
        .overlap_position = OverlapVolume::EOverlapPosition::RightSide,
        .update_order = overlap_update_order,
    },
};


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Ball info
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr SDL_Color ball_color{11, 122, 117, 255};
constexpr float ball_radius = 10.f;
constexpr int ball_update_order = 1;

BallSpecification ball_specs
{
    .position = mid_screen,
    .radius = ball_radius,
    .color = ball_color,
    .update_order = ball_update_order,
};


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Player info
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr SDL_Color player_color{215, 201, 170, 255};
constexpr glm::vec2 player_size{25.f, 100.f};
constexpr int player_update_order = 1;

PlayerSpecification player1
{
    .position = glm::vec2{wall_thickness * 2, mid_screen.y - (player_size.y / 2.f)},
    .size = player_size,
    .color = player_color,
    .player_num = 1,
    .update_order = player_update_order,
};

PlayerSpecification player2
{
    .position = glm::vec2{window_spec.window_size.x - (wall_thickness * 2) - player_size.x, mid_screen.y - (player_size.y / 2.f)},
    .size = player_size,
    .color = player_color,
    .player_num = 2,
    .update_order = player_update_order,
};


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Field info
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr glm::vec2 field_position{25.f, 25.f};
constexpr glm::vec2 field_size{1578.f, 1024.f};
constexpr int field_update_order = -1;  // render behind everything


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Application
////////////////////////////////////////////////////////////////////////////////////////////////////
void seed_c_rand()
{
    std::random_device rd;
    std::array<std::uint32_t, 8> data{};
    for (auto &v : data) v = rd();

    std::seed_seq seq(data.begin(), data.end());
    std::uint32_t seed = 0;
    seq.generate(&seed, &seed +1);

    std::srand(static_cast<unsigned int>(seed));
}

int main()
{
    seed_c_rand();

    Engine app(window_spec);
    std::unique_ptr<GameWorld> world = std::make_unique<GameWorld>(app);

    std::unique_ptr<GameMode> game_mode = std::make_unique<GameMode>(*world, TransformComponent());
    std::unique_ptr<Field> field = std::make_unique<Field>(*world, TransformComponent(field_position), field_size, "data/soccer_field.png");

    // Create all walls
    for (auto const& spec : wall_specs)
    {
        game_mode->spawn_wall(spec);
    }

    // Create all overlaps
    for (auto const& spec : overlap_specs)
    {
        game_mode->spawn_overlap(spec);
    }

    // Create ball
    game_mode->spawn_ball(ball_specs);

    // Create players
    game_mode->spawn_player(player1);
    game_mode->spawn_player(player2);

    // Add objects to world
    world->add_object(std::move(field), field_update_order);
    world->add_object(std::move(game_mode), 1000);

    // Initialize and run application
    app.initialize(std::move(world));
    app.run();

    return 0;
}
