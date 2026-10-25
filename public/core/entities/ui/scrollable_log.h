////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief A self-contained Entity that renders a bounded, scrollable, word-wrapped log of text
///        messages.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_ENTITIES_UI_SCROLLABLE_LOG_H
#define CORE_ENTITIES_UI_SCROLLABLE_LOG_H

#include "core/entities/ui/ui_element.h"

#include <glm/glm.hpp>

#include <cstddef>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

namespace Core::UI
{
class Label;
class ScrollableViewport;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief A self-contained Entity that renders a bounded, scrollable, word-wrapped log of text
///        messages - e.g. a combat/event log.
///
/// ScrollableLog keeps message history as plain data (std::deque<LogItem>, capped at
/// Configuration::max_history) rather than one permanent Label Entity per message. Only the
/// currently-visible messages are backed by live Label Entities, drawn from a small pool sized to
/// the viewport (not the history length) and reused as the scroll position changes, so entity and
/// GPU-texture count stay bounded regardless of how long the log runs.
////////////////////////////////////////////////////////////////////////////////////////////////////
class ScrollableLog : public UIElement
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Font parameters applied to every appended message.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct Font
    {
        /// @brief Path to the font file (relative to data root).
        std::string_view font_path{};
        /// @brief Font size in points.
        int point_size{ 12 };
        /// @brief Text color.
        SDL_Color color{ 255, 255, 255, 255 };
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Construction parameters for ScrollableLog.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct Configuration : UIElement::Configuration
    {
        /// @brief Full size of the log, including the border (if any) and the internal
        ///        ScrollableViewport's scrollbar strip.
        glm::vec2 log_size{ 400, 600 };
        /// @brief Fill color of the log's background panel.
        SDL_Color background_color{ 229, 228, 226, 255 };
        /// @brief Whether the background panel draws a border.
        bool has_border{ true };
        /// @brief Border thickness in pixels, used only when @c has_border is @c true.
        glm::vec2 border_size{ 2, 2 };
        /// @brief Border color, used only when @c has_border is @c true.
        SDL_Color border_color{ 54, 69, 79, 255 };
        /// @brief Font applied to every appended message.
        Font font{};
        /// @brief Padding in pixels applied around each message's text, both horizontally (wrap
        ///        width reduction) and vertically (gap before the next message).
        glm::vec2 text_padding{ 2, 2 };
        /// @brief Maximum number of messages retained. Oldest messages are evicted first.
        std::size_t max_history{ 200 };
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Construction & Destruction
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Constructs a ScrollableLog.
    ///
    /// @param [in] owner  - The Layer or parent Entity that will own this ScrollableLog.
    /// @param [in] config - Optional construction parameters.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ScrollableLog(Entity::Owner owner, Configuration const& config = {});


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Virtual Lifecycle Hook Overrides
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Dirty-checks the viewport's scroll offset and reassigns the pooled Labels to the
    ///        currently-visible message range only when the offset or content height has changed
    ///        since last frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void on_update(float delta_time) override;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Runtime API
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Appends a message to the log, evicting the oldest message if over @c max_history.
    ///        If the log was already scrolled to the bottom, it re-pins to the new bottom;
    ///        otherwise the user's current scroll position is left untouched.
    ///
    /// @param [in] message - Message text. Word-wraps to the log's content width.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void append_log_message(std::string_view message);

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief One retained message: its text and its measured (word-wrapped) slot height,
    ///        including the trailing @c text_padding.y gap before the next message.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct LogItem
    {
        std::string text;
        float height;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Determines the currently-visible LogItem range for @p scroll_offset, grows the
    ///        Label pool if more labels are needed to cover it, and reassigns/hides pool entries
    ///        accordingly.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void recompute_visible_labels(ScrollableViewport& viewport, float scroll_offset);

    /// @brief Font parameters applied to every appended message.
    Font m_font{};

    /// @brief Padding in pixels applied around each message's text.
    glm::vec2 m_text_padding{};

    /// @brief Maximum number of messages retained before the oldest is evicted.
    std::size_t m_max_history{};

    /// @brief Retained message history, oldest-first. Bounded by @c m_max_history.
    std::deque<LogItem> m_history{};

    /// @brief Sum of every LogItem::height currently in @c m_history. Kept incrementally in sync
    ///        by append_log_message() rather than recomputed from scratch.
    float m_total_content_height{ 0.f };

    /// @brief Pool of Label Entities reused to display the currently-visible message range.
    ///        Sized to cover the viewport, not the full history.
    std::vector<std::weak_ptr<Label>> m_label_pool{};

    /// @brief The child ScrollableViewport that hosts @c m_label_pool and owns scroll state.
    std::weak_ptr<ScrollableViewport> m_viewport{};

    /// @brief Forces a recompute on the first on_update() regardless of scroll offset.
    float m_last_scroll_offset{ -1.f };
};

} // namespace Core::UI

#endif // CORE_ENTITIES_UI_SCROLLABLE_LOG_H
