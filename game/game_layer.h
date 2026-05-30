#ifndef GAME_LAYER_H
#define GAME_LAYER_H

#include "aurora_engine_public.h"

class GameLayer : public Core::Layer
{
public:
    explicit GameLayer(Core::Engine& owner);

    ~GameLayer() override = default;

    void initialize_layer() override;
    void update_layer(float delta_time) override;
    void render_layer(SDL_Renderer* renderer) override;
    void cleanup_layer() override;

private:
    constexpr static int wall_thickness{ 25 };
};

#endif // GAME_LAYER_H
