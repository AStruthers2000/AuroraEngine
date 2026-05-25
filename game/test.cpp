#include "aurora_engine_public.h"

#include <print>

class TestComponent : public Core::Component
{
public:
    TestComponent(Core::Entity& owner) : Core::Component(owner)
    {
    }

    virtual ~TestComponent()
    {
        std::println("Test component has been destroyed");
    }

    virtual void initialize_component() override
    {
        std::println("Test component initialized");
    }

    virtual void update_component(float delta_time) override
    {
        std::println("Test component updating");
    }

    virtual void render_component(SDL_Renderer* renderer) override
    {
        std::println("Test component rendering");
    }
};

class TestEntity : public Core::Entity
{
public:
    TestEntity(Core::Layer& owner) : Core::Entity(owner)
    {
        add_component(std::make_unique<TestComponent>(*this));
    }

    virtual ~TestEntity()
    {
        std::println("Test entity has been destroyed");
    }

    virtual void initialize_entity() override
    {
        add_component(std::make_unique<TestComponent>(*this));
        std::println("Test entity initialized");
    }

    virtual void update_entity(float delta_time) override
    {
        static int added{ -1000 };
        if (added++ == 0)
        {
            add_component(std::make_unique<TestComponent>(*this));
        }
        std::println("Test entity updating");
    }

    virtual void render_entity(SDL_Renderer* renderer) override
    {
        std::println("Test entity rendering");
    }
};

class TestLayer : public Core::Layer
{
public:
    explicit TestLayer(Core::Engine& owner) : Core::Layer(owner)
    {
    }

    virtual void initialize_layer() override
    {
        std::println("Test layer initialized");
        add_entity(std::make_unique<TestEntity>(*this));
    }

    virtual void update_layer(float delta_time) override
    {
        std::println("Test layer updating");
    }

    virtual void render_layer(SDL_Renderer* renderer) override
    {
        std::println("Test layer rendering");
    }

    virtual void cleanup_layer() override
    {
        std::println("Test layer cleaning up");
    }
};

int main()
{
    Core::WindowSpecification spec
    {
        .title = "Hello world",
    };

    Core::Engine engine(spec);
    engine.push_layer<TestLayer>();
    engine.run();

    return 0;
}