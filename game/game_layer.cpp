#include "game_layer.h"

#include "wall.h"

#include <format>

//--------------------------------------------------------------------------------------------------
GameLayer::GameLayer(Core::Engine& owner)
    : Core::Layer(owner)
{
    // Add walls
    glm::vec2 window_size = Core::Engine::get().get_window_size();
    float const court_width{ window_size.x };
    float const court_height{ window_size.y };
    add_entity<Wall>(100, glm::vec2{0, 0}, glm::vec2{court_width, wall_thickness});
    add_entity<Wall>(100, glm::vec2{0, court_height - wall_thickness}, glm::vec2{court_width, wall_thickness});
    add_entity<Wall>(100, glm::vec2{0, 0}, glm::vec2{wall_thickness, court_height});
    add_entity<Wall>(100, glm::vec2{court_width - wall_thickness, 0}, glm::vec2{wall_thickness, court_height});

    // Add players

    // Add ball

    // Add field
}

//--------------------------------------------------------------------------------------------------
void GameLayer::initialize()
{

}

//--------------------------------------------------------------------------------------------------
void GameLayer::update(float delta_time)
{

}

//--------------------------------------------------------------------------------------------------
void GameLayer::render(SDL_Renderer* renderer)
{
    SDL_FRect logical_resolution;
    SDL_GetRenderLogicalPresentationRect(renderer, &logical_resolution);

    std::string p1_text = std::format("Player 1: {:>3}", 0);
    std::string p2_text = std::format("Player 2: {:>3}", 0);

    // Debug render text is 8 x 8 pixels
    std::size_t const char_size{ 8 };

    float p1_text_x{ 35.f }; // 25 for wall thickness and 10 for padding
    float text_y{ (25.f / 2.f) - (char_size / 2.f) + 0.5f};

    std::size_t str_len = p2_text.length() * char_size;
    float p2_text_x{ logical_resolution.w - p1_text_x - static_cast<float>(str_len) };

    SDL_SetRenderDrawColor(renderer, 215, 201, 170, 255);
    SDL_RenderDebugText(renderer, p1_text_x, text_y, p1_text.c_str());
    SDL_RenderDebugText(renderer, p2_text_x, text_y, p2_text.c_str());
}

//--------------------------------------------------------------------------------------------------
void GameLayer::cleanup()
{

}
