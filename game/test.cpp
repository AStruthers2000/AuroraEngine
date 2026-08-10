#include "aurora_engine_public.h"

#include "core/components/anchor_component.h"
#include "core/components/rect_render_component.h"
#include "core/components/text_render_component.h"

#include <print>

class PlayerTookDamageEvent : public Core::Event
{
public:
    PlayerTookDamageEvent(int damage) : m_damage(damage) {}

    std::string to_string() const override
    {
        return std::format("Player took {} damage", m_damage);
    }

    EVENT_CLASS_TYPE(PlayerTookDamageEvent)

private:
    int m_damage{ 0 };
};

class TestComponent : public Core::Component
{
public:
    TestComponent(Core::Entity& owner, Core::Component::Configuration const& config) : Core::Component(owner, config)
    {
    }

    virtual ~TestComponent()
    {
        // std::println("Test component has been destroyed");
    }

    virtual void on_event(Core::Event& event) override
    {
        Core::EventDispatcher dispatcher(event);
        dispatcher.dispatch<Core::MouseMovedEvent>(
            [this](Core::MouseMovedEvent& event)
            {
                // std::println("{}", event.to_string());
                return false;
            }
        );
        dispatcher.dispatch<Core::MouseButtonPressedEvent>(
            [this](Core::MouseButtonPressedEvent& event)
            {
                if (event.get_mouse_button() == 1)
                {
                    PlayerTookDamageEvent e(event.get_click_y());
                    broadcast_event(e);
                    return true;
                }
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

    virtual void on_awake() override
    {
        // std::println("Test component initialized");
    }

    virtual void on_update(float delta_time) override
    {
        // std::println("Test component updating");
    }

    virtual void on_late_update(float delta_time) override
    {
        // std::println("Test component late updating");
    }

    virtual void on_fixed_update(float fixed_dt) override
    {
        // std::println("Test component fixed updating");
    }

    virtual void on_render(SDL_Renderer* renderer) override
    {
        // std::println("Test component rendering");
    }

    virtual void on_cleanup() override
    {
        // std::println("Test component cleaning up");
    }
};

class TestEntity : public Core::Entity
{
public:
    TestEntity(Core::Entity::Owner owner, Core::Entity::Configuration const& config) : Core::Entity(owner, config)
    {
        add_component<Core::TransformComponent>("", {.position{ 100, 100 }, .scale{ 1, 1 }, .size{ 100, 100 }});
        add_component<Core::RectRenderComponent>("", {.color{ 100, 100, 100, 255 }});

        SDL_Color text_color{ 255, 0, 0, 255 };
        std::string text = "Hello world, this is some text :)";
        // add_component<Core::TextRenderComponent>("", {.font_path{ Core::Font::TINY_REGULAR }, .point_size{ 25 } , .text{ text }, .color{ text_color }});

        SDL_Color subtitle_color{ 255, 255, 0, 255 };
        // add_component<Core::TextRenderComponent>("subtitle", {.font_path{ Core::Font::TINY_REGULAR }, .point_size{ 25 }, .text{ "Some text" }, .color{ subtitle_color }});

        add_child_entity<Core::UI::Label>({
            .self_anchor = Core::UI::Anchor::CenterLeft,
            .parent_anchor = Core::UI::Anchor::CenterLeft,
            .offset = {5.f, 0.f},
            .font_path = Core::Font::TINY_REGULAR,
            .point_size = 25,
            .text = text,
            .color = text_color,
        }, "health_label");

        add_child_entity<Core::UI::Label>({
            .offset = {0.f, 0.f},
            .font_path = Core::Font::TINY_REGULAR,
            .point_size = 25,
            .text = "Some text",
            .color = subtitle_color
        });
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
        dispatcher.dispatch<PlayerTookDamageEvent>(
            [this](PlayerTookDamageEvent& event)
            {
                if (auto text_ptr = get_child_entity<Core::UI::Label>("health_label").lock())
                {
                    text_ptr->set_text(event.to_string());

                    auto rand_color = []()
                    {
                        return static_cast<std::uint8_t>(rand() * 255.f);
                    };
                    SDL_Color new_color(rand_color(), rand_color(), rand_color(), 255);
                    text_ptr->set_color(new_color);
                }
                return true;
            }
        );
    }

    virtual void on_awake() override
    {
        add_component<TestComponent>("");

        add_child_entity<Core::UI::Label>({
            .offset = {0.f, -30.f},
            .font_path = Core::Font::TINY_REGULAR,
            .point_size = 20,
            .text = "I am a child LabelEntity",
            .color = {255, 255, 255, 255}
        });
        // std::println("Test entity initialized");
    }

    virtual void on_update(float delta_time) override
    {
        // std::println("Test entity updating");
    }

    virtual void on_late_update(float delta_time) override
    {
        // std::println("Test entity late updating");
    }

    virtual void on_fixed_update(float fixed_dt) override
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

    virtual void on_render(SDL_Renderer* renderer) override
    {
        // std::println("Test entity rendering");
    }

    virtual void on_cleanup() override
    {
        // std::println("Test entity cleaning up");
    }

private:
    glm::vec2 move_dir{ 0.f, 0.f };
};

// Showcases Core::UI::AnchorComponent: a background box with several Labels pinned to its corners
// and center. Each Label's own bounding-box pivot (self_anchor) is matched to the same point on
// the box's bounding box (parent_anchor), so the label stays flush against that point regardless
// of the box's size; `offset` then nudges it inward as padding.
class AnchorDemoBox : public Core::Entity
{
public:
    AnchorDemoBox(Core::Entity::Owner owner, Core::Entity::Configuration const& config) : Core::Entity(owner, config)
    {
        add_component<Core::TransformComponent>("", { .position{ 400.f, 300.f }, .size{ 300.f, 200.f } });
        add_component<Core::RectRenderComponent>("", { .color{ 40, 40, 60, 255 } });
    }

    virtual void on_awake() override
    {
        add_anchored_label(Core::UI::Anchor::TopLeft,     "Top Left",     {  8.f,  8.f });
        add_anchored_label(Core::UI::Anchor::TopRight,    "Top Right",    { -8.f,  8.f });
        add_anchored_label(Core::UI::Anchor::Center,      "Center",       {  0.f,  0.f });
        add_anchored_label(Core::UI::Anchor::BottomLeft,  "Bottom Left",  {  8.f, -8.f });
        add_anchored_label(Core::UI::Anchor::BottomRight, "Bottom Right", { -8.f, -8.f });
    }

private:
    void add_anchored_label(Core::UI::Anchor anchor, std::string_view text, glm::vec2 offset)
    {
        add_child_entity<Core::UI::Label>({
            .self_anchor = anchor,
            .parent_anchor = anchor,
            .offset = offset,
            .font_path = Core::Font::TINY_REGULAR,
            .point_size = 14,
            .text = text,
            .color = { 255, 255, 255, 255 },
        });
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
        dispatcher.dispatch<Core::WindowCloseEvent>(
            [this](Core::WindowCloseEvent& event)
            {
                Core::Engine::get().stop();
                return true;
            }
        );
    }

    virtual void on_initialize() override
    {
        // std::println("Test layer initialized");
        add_entity<TestEntity>();
        add_entity<AnchorDemoBox>();
    }

    virtual void on_update(float delta_time) override
    {
        // std::println("Test layer updating");
    }

    virtual void on_late_update(float delta_time) override
    {
        // std::println("Test layer late updating");
    }

    virtual void on_fixed_update(float fixed_dt) override
    {
        // std::println("Test layer fixed updating");
    }

    virtual void on_render(SDL_Renderer* renderer) override
    {
        // std::println("Test layer rendering");
    }

    virtual void on_cleanup() override
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
    spec.logical_size *= 4;

    Core::Engine engine(spec);
    engine.set_fixed_timestep(1.f/144.f);
    engine.push_layer<TestLayer>();
    engine.run();

    return 0;
}