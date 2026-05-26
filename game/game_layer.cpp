#include "game_layer.h"

#include "wall.h"

#include <format>

//--------------------------------------------------------------------------------------------------
GameLayer::GameLayer(Core::Engine& owner)
    : Core::Layer(owner)
{
    // Add walls
    add_entity(std::make_unique<Wall>(*this, 100, glm::vec2{0, 0}, glm::vec2{100, 100}));
    add_entity(std::make_unique<Wall>(*this, 100, glm::vec2{1000, 0}, glm::vec2{10, 100}));

    // Add players

    // Add ball

    // Add field
}

//--------------------------------------------------------------------------------------------------
void GameLayer::initialize_layer()
{

}

//--------------------------------------------------------------------------------------------------
void GameLayer::update_layer(float delta_time)
{

}

//--------------------------------------------------------------------------------------------------
void GameLayer::render_layer(SDL_Renderer* renderer)
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
void GameLayer::cleanup_layer()
{

}
