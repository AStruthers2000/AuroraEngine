#include "player.h"

using namespace AuroraEngine;

int main(int argc, char* argv[])
{
    WindowSpecification window_spec
    {
        .window_size = {1920, 1080},
        .logical_size = {1920, 1080},
        .title = "Aurora Engine Demo",
    };

    Engine app(window_spec);
    std::unique_ptr<GameWorld> world = std::make_unique<GameWorld>(app);

    std::unique_ptr<TestGame::Player> player = std::make_unique<TestGame::Player>(*world);
    world->add_object(std::move(player), 0);

    app.initialize(std::move(world));
    app.run();
}
