////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/entities/ui/scrollable_log.h"

#include "core/asset_manager.h"
#include "core/components/transform_component.h"
#include "core/entities/ui/label.h"
#include "core/entities/ui/panel.h"
#include "core/entities/ui/scrollable_viewport.h"

#include <algorithm>
#include <string>

namespace Core::UI
{

//--------------------------------------------------------------------------------------------------
ScrollableLog::ScrollableLog(Entity::Owner owner, Configuration const& config)
    : UIElement(owner, config)
    , m_font(config.font)
    , m_text_padding(config.text_padding)
    , m_max_history(config.max_history)
{
    TransformComponent::Configuration transform_cfg;
    transform_cfg.position = config.offset;
    transform_cfg.size = config.log_size;
    add_component<TransformComponent>("", transform_cfg);

    Panel::Configuration log_cfg;
    log_cfg.update_order  = config.update_order;
    log_cfg.self_anchor   = config.self_anchor;
    log_cfg.parent_anchor = config.parent_anchor;
    log_cfg.offset        = { 0, 0 };
    log_cfg.panel_size    = config.log_size;
    log_cfg.panel_color   = config.background_color;
    log_cfg.has_border    = config.has_border;
    log_cfg.border_size   = config.border_size;
    log_cfg.border_color  = config.border_color;
    add_child_entity<Panel>(log_cfg, "main-body");

    glm::vec2 const inset = config.has_border ? config.border_size : glm::vec2{ 0.f, 0.f };

    ScrollableViewport::Configuration viewport_cfg;
    viewport_cfg.self_anchor   = Anchor::TopLeft;
    viewport_cfg.parent_anchor = Anchor::TopLeft;
    viewport_cfg.offset        = inset;
    viewport_cfg.viewport_size = config.log_size - (inset * 2.f);
    m_viewport = add_child_entity<ScrollableViewport>(viewport_cfg, "viewport");
}

//--------------------------------------------------------------------------------------------------
void ScrollableLog::on_update(float delta_time)
{
    auto viewport = m_viewport.lock();
    if (!viewport)
    {
        return;
    }

    float const current_offset = viewport->get_scroll_offset();
    if (current_offset == m_last_scroll_offset)
    {
        return;
    }

    m_last_scroll_offset = current_offset;
    recompute_visible_labels(*viewport, current_offset);
}

//--------------------------------------------------------------------------------------------------
void ScrollableLog::append_log_message(std::string_view message)
{
    auto viewport = m_viewport.lock();
    if (!viewport)
    {
        return;
    }

    bool const was_at_bottom = viewport->get_scroll_offset() >= viewport->get_max_scroll_offset();

    float const wrap_width = std::max(1.f, viewport->get_content_width() - (m_text_padding.x * 2.f));
    glm::vec2 const measured = AssetManager::instance().measure_text(
        m_font.font_path, m_font.point_size, message, static_cast<int>(wrap_width));

    float const item_height = measured.y + m_text_padding.y;
    m_history.push_back(LogItem{ std::string(message), item_height });
    m_total_content_height += item_height;

    while (m_history.size() > m_max_history)
    {
        m_total_content_height -= m_history.front().height;
        m_history.pop_front();
    }

    viewport->set_content_height(m_total_content_height);

    if (was_at_bottom)
    {
        viewport->set_scroll_offset(viewport->get_max_scroll_offset());
    }

    // Force on_update() to recompute even if the scroll offset itself didn't change (e.g. the
    // log wasn't scrolled to the bottom and content grew below the visible range).
    m_last_scroll_offset = -1.f;
}

//--------------------------------------------------------------------------------------------------
void ScrollableLog::recompute_visible_labels(ScrollableViewport& viewport, float scroll_offset)
{
    float const viewport_height = viewport.get_viewport_size().y;
    float const wrap_width = std::max(1.f, viewport.get_content_width() - (m_text_padding.x * 2.f));

    std::vector<std::pair<std::size_t, float>> visible_items{};
    float running_offset = 0.f;
    for (std::size_t i = 0; i < m_history.size(); ++i)
    {
        float const item_top = running_offset;
        float const item_bottom = running_offset + m_history[i].height;

        if (item_bottom > scroll_offset && item_top < scroll_offset + viewport_height)
        {
            visible_items.emplace_back(i, item_top - scroll_offset);
        }

        running_offset = item_bottom;
    }

    // Grow the pool if more labels are needed to cover the visible range - bounded by viewport
    // size, not history length, since only ever as many labels exist as fit on screen at once.
    while (m_label_pool.size() < visible_items.size())
    {
        Label::Configuration label_cfg;
        label_cfg.self_anchor   = Anchor::TopLeft;
        label_cfg.parent_anchor = Anchor::TopLeft;
        label_cfg.font_path     = m_font.font_path;
        label_cfg.point_size    = m_font.point_size;
        label_cfg.color         = m_font.color;
        label_cfg.wrap_width    = static_cast<int>(wrap_width);
        std::string const tag   = "pool-" + std::to_string(m_label_pool.size());
        m_label_pool.push_back(viewport.add_child_entity<Label>(label_cfg, tag));
    }

    for (std::size_t pool_index = 0; pool_index < m_label_pool.size(); ++pool_index)
    {
        auto label = m_label_pool[pool_index].lock();
        if (!label)
        {
            continue;
        }

        if (pool_index < visible_items.size())
        {
            auto const& [history_index, local_y] = visible_items[pool_index];
            label->set_text(m_history[history_index].text);
            label->set_wrap_width(static_cast<int>(wrap_width));
            label->set_offset({ m_text_padding.x, local_y });
            label->set_active();
        }
        else
        {
            // Inactive entities still render - fully disable to actually stop drawing stale text.
            label->set_disabled();
        }
    }
}

} // namespace Core::UI

