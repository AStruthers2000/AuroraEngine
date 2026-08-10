////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Renders a single line of text to the screen using the sibling TransformComponent for
///        position. Font face and size are fixed at construction; text and color can be changed
///        at runtime.
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
/// @brief Renders a single line of text during the render pass of each frame.
///
/// TextRenderComponent rasterizes a UTF-8 text string into an SDL_Texture using SDL_ttf. The font
/// and point size are fixed at construction. Text content and color can be updated at runtime via
/// set_text() and set_color(); the backing GPU texture is rebuilt lazily on the next on_render()
/// call, so successive runtime changes in one frame cost only one re-rasterize.
///
/// @note  Requires a sibling TransformComponent on the same Entity. The transform's world position
///        is used as the top-left corner of the rendered text.
///
/// @note  Font loading is delegated to the AssetManager, so the same TTF_Font object is shared
///        across all components that request the same path + point size.
////////////////////////////////////////////////////////////////////////////////////////////////////
class TextRenderComponent : public RenderComponent
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Construction parameters for TextRenderComponent.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct Configuration : public RenderComponent::Configuration
    {
        /// @brief Path to the font file (relative to data root).
        std::string_view font_path{};
        /// @brief Font size in points.
        int              point_size{ 12 };
        /// @brief Initial text string to display.
        std::string_view text{};
        /// @brief Initial text color (RGBA).
        SDL_Color        color{ 255, 255, 255, 255 };
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Construction & Destruction
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Constructs a TextRenderComponent.
    ///
    /// @param [in] owner  - The Entity that owns this Component.
    /// @param [in] config - Optional construction parameters (font path, point size, text, color).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    explicit TextRenderComponent(Entity& owner, Configuration const& config = {});

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Destructor. Default - GPU texture is released in on_cleanup().
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ~TextRenderComponent() override = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Virtual Lifecycle Hook Overrides
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Loads the font from the AssetManager and caches a weak_ptr to the sibling
    ///        TransformComponent. Called once during the Entity's awake phase.
    ///
    /// @note  Asserts that a TransformComponent sibling exists and that the font can be loaded.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void on_awake() override;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Renders the current text string using the sibling TransformComponent's world
    ///        position as the top-left corner. Rebuilds the backing SDL_Texture lazily if text
    ///        or color has changed since the last call.
    ///
    /// @param [in] renderer - The SDL renderer for the current frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void on_render(SDL_Renderer* renderer) override;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Destroys the cached SDL_Texture. Called once before the owning Entity is destroyed.
    ///
    /// @note  The GPU texture must be released explicitly; SDL does not garbage-collect textures.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void on_cleanup() override;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Runtime API
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Replaces the displayed text. The backing SDL_Texture is rebuilt lazily on the next
    ///        on_render() call.
    ///
    /// @param [in] text - New text string. Single line only; newlines are not supported.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_text(std::string_view text);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The current text string.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::string_view get_text() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Changes the text color. The backing SDL_Texture is rebuilt lazily on the next
    ///        on_render() call.
    ///
    /// @param [in] color - New RGBA text color.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_color(SDL_Color color);

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Destroys the existing SDL_Texture (if any) and rasterizes @c m_text into a new one
    ///        using @c m_font and @c m_color. No-ops if the font weak_ptr has expired or
    ///        @c m_text is empty.
    ///
    /// @param [in] renderer - SDL renderer used to upload the rasterized surface as a GPU texture.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void rebuild_texture(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Measures @c m_text with @c m_font via TTF_GetStringSize() (no GPU texture involved)
    ///        and pushes the result into the sibling TransformComponent's size. Called whenever
    ///        the text content changes, so layout/anchoring code always sees an up-to-date size -
    ///        independent of the lazily-rebuilt GPU texture in rebuild_texture().
    ///
    /// @note  No-ops if the font weak_ptr has expired or the sibling TransformComponent is gone.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void measure_and_apply_size();

    /// @brief Font file path stored for AssetManager lookup.
    std::string  m_font_path{};

    /// @brief Font point size.
    int m_point_size{};

    /// @brief Shared font handle from the AssetManager.
    std::weak_ptr<TTF_Font> m_font{};

    /// @brief Cached sibling TransformComponent.
    std::weak_ptr<TransformComponent> m_owning_transform{};

    /// @brief Current text string to render.
    std::string m_text{};

    /// @brief Current text color.
    SDL_Color m_color{};

    /// @brief Lazily built GPU texture.
    SDL_Texture* m_cached_texture{ nullptr };

    /// @brief Pixel width of the cached texture.
    float m_cached_texture_width{};

    /// @brief Pixel height of the cached texture.
    float m_cached_texture_height{};

    /// @brief False when text or color has changed.
    bool m_cache_valid{ false };
};

} // namespace Core

#endif // CORE_COMPONENTS_TEXT_RENDER_COMPONENT_H
