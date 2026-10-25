////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/components/text_render_component.h"

#include "core/asset_manager.h"
#include "core/sdl_color_ops.h"
#include "core/components/transform_component.h"

namespace Core
{

//--------------------------------------------------------------------------------------------------
TextRenderComponent::TextRenderComponent(Entity& owner, Configuration const& config)
    : RenderComponent(owner, config)
    , m_font_path(config.font_path)
    , m_point_size(config.point_size)
    , m_text(config.text)
    , m_color(config.color)
    , m_wrap_width(config.wrap_width)
{
}

//--------------------------------------------------------------------------------------------------
void TextRenderComponent::on_awake()
{
    m_owning_transform = get_sibling_component<TransformComponent>();
    m_font = AssetManager::instance().get_font(m_font_path, m_point_size);
    measure_and_apply_size();
}

//--------------------------------------------------------------------------------------------------
void TextRenderComponent::on_render(SDL_Renderer* renderer)
{
    if (!m_cache_valid)
    {
        rebuild_texture(renderer);
        m_cache_valid = true;
    }

    if (!m_cached_texture)
    {
        return;
    }

    auto transform = m_owning_transform.lock();
    if (!transform)
    {
        return;
    }

    SDL_FRect dst
    {
        .x = transform->get_world_position().x,
        .y = transform->get_world_position().y,
        .w = m_cached_texture_width,
        .h = m_cached_texture_height,
    };
    SDL_RenderTexture(renderer, m_cached_texture, nullptr, &dst);
}

//--------------------------------------------------------------------------------------------------
void TextRenderComponent::on_cleanup()
{
    if (m_cached_texture)
    {
        SDL_DestroyTexture(m_cached_texture);
        m_cached_texture = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
void TextRenderComponent::set_text(std::string_view text)
{
    if (text != m_text)
    {
        m_text = text;
        m_cache_valid = false;
        measure_and_apply_size();
    }
}

//--------------------------------------------------------------------------------------------------
std::string_view TextRenderComponent::get_text() const
{
    return m_text;
}

//--------------------------------------------------------------------------------------------------
void TextRenderComponent::set_color(SDL_Color color)
{
    if (color != m_color)
    {
        m_color = color;
        m_cache_valid = false;
    }
}

//--------------------------------------------------------------------------------------------------
void TextRenderComponent::set_wrap_width(int wrap_width)
{
    if (wrap_width != m_wrap_width)
    {
        m_wrap_width = wrap_width;
        m_cache_valid = false;
        measure_and_apply_size();
    }
}

//--------------------------------------------------------------------------------------------------
void TextRenderComponent::rebuild_texture(SDL_Renderer* renderer)
{
    if (auto font_ptr = m_font.lock())
    {
        SDL_Surface* surface{ nullptr };
        SDL_Texture* texture{ nullptr };

        surface = TTF_RenderText_Blended_Wrapped(font_ptr.get(), m_text.c_str(), 0, m_color, scaled_wrap_width());
        if (surface)
        {
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_DestroySurface(surface);
        }

        if (texture)
        {
            float w, h;
            SDL_GetTextureSize(texture, &w, &h);
            SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);

            // Destroy cached texture before reassigning so that the old cached texture is properly
            // freed
            SDL_DestroyTexture(m_cached_texture);

            m_cached_texture = texture;
            m_cached_texture_width = glm::round(w * Core::AssetManager::INVERSE_FONT_UPSCALING_FACTOR);
            m_cached_texture_height = glm::round(h * Core::AssetManager::INVERSE_FONT_UPSCALING_FACTOR);
            m_cache_valid = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
void TextRenderComponent::measure_and_apply_size()
{
    auto transform = m_owning_transform.lock();
    if (!transform)
    {
        return;
    }

    glm::vec2 const size = AssetManager::instance().measure_text(m_font_path, m_point_size, m_text, m_wrap_width);
    if (size != glm::vec2{ 0.f, 0.f })
    {
        transform->set_size(size);
    }
}

//--------------------------------------------------------------------------------------------------
int TextRenderComponent::scaled_wrap_width() const
{
    return m_wrap_width > 0
        ? static_cast<int>(glm::round(static_cast<float>(m_wrap_width) * Core::AssetManager::FONT_UPSCALING_FACTOR))
        : 0;
}

} // namespace Core