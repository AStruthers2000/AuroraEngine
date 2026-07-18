#include "aurora_engine_public.h"

#include "core/components/render_components/rect_render_component.h"

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

    virtual void on_event(Core::Event& event)
    {
        Core::EventDispatcher dispatcher(event);
        dispatcher.dispatch<Core::MouseMovedEvent>(
            [this](Core::MouseMovedEvent& event)
            {
                // std::println("{}", event.to_string());
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
        glm::vec2 position{ 100, 100 };
        glm::vec2 scale{ 100, 100 };
        add_component<Core::TransformComponent>(position, scale);

        Core::Component::Order order{};
        SDL_Color color{ 100, 100, 100, 255 };
        add_component<Core::RectRenderComponent>(order, color);
    }

    virtual ~TestEntity()
    {
        // std::println("Test entity has been destroyed");
    }

    virtual void on_event(Core::Event& event)
    {
        Core::EventDispatcher dispatcher(event);
        dispatcher.dispatch<Core::KeyPressedEvent>(
            [this](Core::KeyPressedEvent& event)
            {
                if (!event.is_repeat())
                {   
                    switch (event.get_scancode())
                    {
                        case SDL_Scancode::SDL_SCANCODE_A:
                            move_dir.x -= 1.f;
                            break;
                        case SDL_Scancode::SDL_SCANCODE_D:
                            move_dir.x += 1.f;
                            break;
                        case SDL_Scancode::SDL_SCANCODE_W:
                            move_dir.y -= 1.f;
                            break;
                        case SDL_Scancode::SDL_SCANCODE_S:
                            move_dir.y += 1.f;
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
                        move_dir.x += 1.f;
                        break;
                    case SDL_Scancode::SDL_SCANCODE_D:
                        move_dir.x -= 1.f;
                        break;
                    case SDL_Scancode::SDL_SCANCODE_W:
                        move_dir.y += 1.f;
                        break;
                    case SDL_Scancode::SDL_SCANCODE_S:
                        move_dir.y -= 1.f;
                        break;
                    default:
                        break;
                }
                return false;
            }
        );
        dispatcher.dispatch<Core::MouseButtonPressedEvent>(
            [this](Core::MouseButtonPressedEvent& event)
            {
                std::println("Entity handling mouse button pressed event: {}", event.to_string());
                return true;
            }
        );
    }

    virtual void awake() override
    {
        add_component<TestComponent>();
        // std::println("Test entity initialized");
    }

    virtual void update(float delta_time) override
    {
        // std::println("Test entity updating");
    }

    virtual void late_update(float delta_time) override
    {
        // std::println("Test entity late updating");
    }

    virtual void fixed_update(float fixed_dt) override
    {
        // std::println("Test entity fixed updating");
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

            glm::vec2 vel = norm * 500.f * fixed_dt;
            if (auto transform = get_component<Core::TransformComponent>().lock())
            {
                transform->set_velocity(vel);
                transform->update_position(transform->get_velocity());
            }             
        }
    }

    virtual void render(SDL_Renderer* renderer) override
    {
        // std::println("Test entity rendering");
    }

    virtual void cleanup() override
    {
        // std::println("Test entity cleaning up");
    }

private:
    glm::vec2 move_dir{ 0.f, 0.f };
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
        dispatcher.dispatch<Core::WindowCloseEvent>(
            [this](Core::WindowCloseEvent& event)
            {
                Core::Engine::get().stop();
                return true;
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
};

int main()
{
    Core::WindowSpecification spec
    {
        .title = "Hello world",
    };

    Core::Engine engine(spec);
    engine.set_fixed_timestep(1.f/144.f);
    engine.push_layer<TestLayer>();
    engine.run();

    return 0;
}