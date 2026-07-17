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

    virtual void awake() override
    {
        std::println("Test component initialized");
    }

    virtual void update(float delta_time) override
    {
        std::println("Test component updating");
    }

    virtual void late_update(float delta_time) override
    {
        std::println("Test component late updating");
    }

    virtual void fixed_update(float fixed_dt) override
    {
        std::println("Test component fixed updating");
    }

    virtual void render(SDL_Renderer* renderer) override
    {
        std::println("Test component rendering");
    }

    virtual void cleanup() override
    {
        std::println("Test component cleaning up");
    }
};

class TestEntity : public Core::Entity
{
public:
    TestEntity(Core::Layer& owner) : Core::Entity(owner)
    {
        add_component<TestComponent>();
    }

    virtual ~TestEntity()
    {
        std::println("Test entity has been destroyed");
    }

    virtual void awake() override
    {
        add_component<TestComponent>();
        std::println("Test entity initialized");
    }

    virtual void update(float delta_time) override
    {
        static int added{ -1000 };
        if (added++ == 0)
        {
            add_component<TestComponent>();
        }
        std::println("Test entity updating");
    }

    virtual void late_update(float delta_time) override
    {
        std::println("Test entity late updating");
    }

    virtual void fixed_update(float fixed_dt) override
    {
        std::println("Test entity fixed updating");
    }

    virtual void render(SDL_Renderer* renderer) override
    {
        std::println("Test entity rendering");
    }

    virtual void cleanup() override
    {
        std::println("Test entity cleaning up");
    }
};

class TestLayer : public Core::Layer
{
public:
    explicit TestLayer(Core::Engine& owner) : Core::Layer(owner)
    {
    }

    virtual void initialize() override
    {
        std::println("Test layer initialized");
        add_entity<TestEntity>();
    }

    virtual void update(float delta_time) override
    {
        std::println("Test layer updating");
    }

    virtual void late_update(float delta_time) override
    {
        std::println("Test layer late updating");
    }

    virtual void fixed_update(float fixed_dt) override
    {
        std::println("Test layer fixed updating");
    }

    virtual void render(SDL_Renderer* renderer) override
    {
        std::println("Test layer rendering");
    }

    virtual void cleanup() override
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