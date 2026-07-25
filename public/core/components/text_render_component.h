////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Renders a single line of text to the screen during the render phase of each frame.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_COMPONENTS_TEXT_RENDER_COMPONENT_H
#define CORE_COMPONENTS_TEXT_RENDER_COMPONENT_H

#include "core/components/render_component.h"

#include <SDL3_ttf/SDL_ttf.h>

#include <memory>
#include <string>
#include <string_view>

namespace Core
{

class TransformComponent;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Renders a single line of text to the screen during the render phase of each frame. Font
///        face and point size are fixed at construction. Text content and color may be changed at
///        runtime via set_text() and set_color(); the backing texture is rebuilt lazily on the next
///        render() call.
////////////////////////////////////////////////////////////////////////////////////////////////////
class TextRenderComponent : public RenderComponent
{
public:
    struct Configuration : public RenderComponent::Configuration
    {
        std::string_view font_path{};
        int point_size{ 12 };
        std::string_view text{};
        SDL_Color color{ 255, 255, 255, 255 };
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Constructs a TextRenderComponent.
    ///
    /// @param [in] owning_entity   - The Entity that owns this Component instance. Passed to the
    ///                               parent constructor.
    /// @param [in] component_order - The sorting order mapping this Component will follow. Passed
    ///                               to the parent constructor.
    /// @param [in] font_path       - Path to the font file, relative to the data root. Use a
    ///                               constant from the Core::Font namespace.
    /// @param [in] point_size      - Desired display size of the font in points.
    /// @param [in] text            - Initial text string to render. Single line only.
    /// @param [in] color           - RGBA color of the rendered text.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    explicit TextRenderComponent(Entity& owner, Configuration const& config = {});

    ~TextRenderComponent() override = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Initializes this component by fetching the font from the AssetManager and caching
    ///        a reference to the sibling TransformComponent.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void awake() override;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Renders the text string to the screen. Uses the sibling TransformComponent for
    ///        position. Rebuilds the backing texture if text or color has changed since the last
    ///        render call.
    ///
    /// @param [in] renderer - Renderer provided by the owning Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void render(SDL_Renderer* renderer) override;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Cleans up the text render component.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void cleanup() override;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Replaces the displayed text. The backing texture is rebuilt on the next render()
    ///        call.
    ///
    /// @param [in] text - New text string. Single line only.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_text(std::string_view text);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The current text string.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::string_view get_text() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Changes the text color. The backing texture is rebuilt on the next render() call.
    ///
    /// @param [in] color - New RGBA text color.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_color(SDL_Color color);

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Destroy the existing texture (if any) and rasterize m_text into a new one. No-ops if
    ///        the font weak_ptr has expired or m_text is empty.
    ///
    /// @param [in] renderer - SDL renderer used to create the GPU texture.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void rebuild_texture(SDL_Renderer* renderer);

    std::string m_font_path{};
    int m_point_size{};
    std::weak_ptr<TTF_Font> m_font{};

    std::weak_ptr<TransformComponent> m_owning_transform{};

    std::string m_text{};
    SDL_Color m_color{};

    SDL_Texture* m_cached_texture{ nullptr };
    float m_cached_texture_width{};
    float m_cached_texture_height{};
    bool m_cache_valid{ false };
};

} // namespace Core

#endif // CORE_COMPONENTS_TEXT_RENDER_COMPONENT_H