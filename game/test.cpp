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
        // std::println("Test component has been destroyed");
    }

    virtual void awake() override
    {
        // std::println("Test component initialized");
    }

    virtual void update(float delta_time) override
    {
        // std::println("Test component updating");
    }

    virtual void late_update(float delta_time) override
    {
        // std::println("Test component late updating");
    }

    virtual void fixed_update(float fixed_dt) override
    {
        // std::println("Test component fixed updating");
    }

    virtual void render(SDL_Renderer* renderer) override
    {
        // std::println("Test component rendering");
    }

    virtual void cleanup() override
    {
        // std::println("Test component cleaning up");
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
        // std::println("Test entity has been destroyed");
    }

    virtual void awake() override
    {
        add_component<TestComponent>();
        // std::println("Test entity initialized");
    }

    virtual void update(float delta_time) override
    {
        // static int added{ -1000 };
        // if (added++ == 0)
        // {
        //     add_component<TestComponent>();
        // }
        // std::println("Test entity updating");
    }

    virtual void late_update(float delta_time) override
    {
        // std::println("Test entity late updating");
    }

    virtual void fixed_update(float fixed_dt) override
    {
        // std::println("Test entity fixed updating");
    }

    virtual void render(SDL_Renderer* renderer) override
    {
        // std::println("Test entity rendering");
    }

    virtual void cleanup() override
    {
        // std::println("Test entity cleaning up");
    }
};

class TestLayer : public Core::Layer
{
public:
    explicit TestLayer(Core::Engine& owner) : Core::Layer(owner)
    {
    }

    virtual void on_event(Core::Event& event)
    {
        Core::EventDispatcher dispatcher(event);
        dispatcher.dispatch<Core::WindowResizeEvent>(
            [this](Core::WindowResizeEvent& event)
            {
                std::println("{}", event.to_string());
                return false;
            }
        );
        dispatcher.dispatch<Core::KeyPressedEvent>(
            [this](Core::KeyPressedEvent& event)
            {
                if (!event.is_repeat())
                {   
                    switch (event.get_scancode())
                    {
                        case SDL_Scancode::SDL_SCANCODE_A:
                            std::println("Moving left");
                            move_dir.x -= 1.f;
                            break;
                        case SDL_Scancode::SDL_SCANCODE_D:
                            std::println("Moving right");
                            move_dir.x += 1.f;
                            break;
                        case SDL_Scancode::SDL_SCANCODE_W:
                            std::println("Moving up");
                            move_dir.y += 1.f;
                            break;
                        case SDL_Scancode::SDL_SCANCODE_S:
                            std::println("Moving down");
                            move_dir.y -= 1.f;
                            break;
                        default:
                            break;
                    }
                }
                return false;
            }
        );
        dispatcher.dispatch<Core::KeyReleasedEvent>(
            [this](Core::KeyReleasedEvent& event)
            {
                switch (event.get_scancode())
                {
                    case SDL_Scancode::SDL_SCANCODE_A:
                        std::println("Done moving left");
                        move_dir.x += 1.f;
                        break;
                    case SDL_Scancode::SDL_SCANCODE_D:
                        std::println("Done moving right");
                        move_dir.x -= 1.f;
                        break;
                    case SDL_Scancode::SDL_SCANCODE_W:
                        std::println("Done moving up");
                        move_dir.y -= 1.f;
                        break;
                    case SDL_Scancode::SDL_SCANCODE_S:
                        std::println("Done moving down");
                        move_dir.y += 1.f;
                        break;
                    default:
                        break;
                }
                return false;
            }
        );
        dispatcher.dispatch<Core::MouseMovedEvent>(
            [this](Core::MouseMovedEvent& event)
            {
                std::println("{}", event.to_string());
                return false;
            }
        );
        dispatcher.dispatch<Core::MouseButtonPressedEvent>(
            [this](Core::MouseButtonPressedEvent& event)
            {
                std::println("{}", event.to_string());
                return false;
            }
        );
        dispatcher.dispatch<Core::MouseButtonReleasedEvent>(
            [this](Core::MouseButtonReleasedEvent& event)
            {
                std::println("{}", event.to_string());
                return false;
            }
        );
    }

    virtual void initialize() override
    {
        // std::println("Test layer initialized");
        add_entity<TestEntity>();
    }

    virtual void update(float delta_time) override
    {
        // std::println("Test layer updating");
        float length = glm::length(move_dir);
        if (length >= 0.001)
        {
            static glm::vec2 last_norm{ 0.f, 0.f };
            glm::vec2 norm = glm::normalize(move_dir);
            if (norm != last_norm)
            {
                std::println("Move dir = ({}, {})", norm.x, norm.y);
            }
            last_norm = norm;
        }
    }

    virtual void late_update(float delta_time) override
    {
        // std::println("Test layer late updating");
    }

    virtual void fixed_update(float fixed_dt) override
    {
        // std::println("Test layer fixed updating");
    }

    virtual void render(SDL_Renderer* renderer) override
    {
        // std::println("Test layer rendering");
    }

    virtual void cleanup() override
    {
        // std::println("Test layer cleaning up");
    }

private:
    glm::vec2 move_dir{ 0.f, 0.f };
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