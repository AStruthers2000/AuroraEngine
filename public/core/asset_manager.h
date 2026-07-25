////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Shared resource manager for font assets loaded via SDL_ttf.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_ASSET_MANAGER_H
#define CORE_ASSET_MANAGER_H

#include <SDL3_ttf/SDL_ttf.h>

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace Core
{

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Compile-time path constants for every bundled font face.
///
/// Pass these constants to TextRenderComponent::Configuration::font_path or to
/// AssetManager::load_font() to load a font from the data directory.
///
/// @note  Paths are relative to the data root set via AssetManager::set_data_root().
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Font
{
constexpr std::string_view COLLEGE_REGULAR          = "fonts/college/college.ttf";
constexpr std::string_view COLLEGE_BOLD             = "fonts/college/collegeb.ttf";
constexpr std::string_view COLLEGE_CONDENSED        = "fonts/college/collegec.ttf";
constexpr std::string_view COLLEGE_SEMI_CONDENSED   = "fonts/college/colleges.ttf";
constexpr std::string_view GO_LONG_REGULAR          = "fonts/golong/golong.ttf";
constexpr std::string_view HIT_THE_ROAD_REGULAR     = "fonts/hitroad/hitroad.ttf";
constexpr std::string_view LED_REAL_REGULAR         = "fonts/led_real/led_real.ttf";
constexpr std::string_view LED_REAL_ITALIC          = "fonts/led_real/ledreali.ttf";
constexpr std::string_view LED_REAL_SPEAK_AND_SPELL = "fonts/led_real/led_sas.ttf";
constexpr std::string_view LIBBY_REGULAR            = "fonts/libby/LIBBY.TTF";
constexpr std::string_view ONE_FORTY_SEVEN_REGULAR  = "fonts/one47/one47.ttf";
constexpr std::string_view PROPAGANDA_REGULAR       = "fonts/propagnd/propaganda.ttf";
constexpr std::string_view TINY_REGULAR             = "fonts/tiny/tiny.ttf";
constexpr std::string_view WHITE_RABBIT_REGULAR     = "fonts/whitrabt/whitrabt.ttf";
} // namespace Font

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Singleton resource manager for font assets.
///
/// AssetManager loads TTF_Font objects from disk on demand, shares them as std::shared_ptr, and
/// exposes them to callers as std::weak_ptr. Loaded fonts are reference-counted: a font remains
/// alive as long as at least one std::shared_ptr to it exists (e.g. held by a
/// TextRenderComponent).
///
/// Fonts are rendered at FONT_UPSCALING_FACTOR times the requested point size and displayed at
/// INVERSE_FONT_UPSCALING_FACTOR scale. This oversampling produces sharper glyphs when the
/// rendered output is smaller than the rasterized size.
///
/// @note  Must call set_data_root() once at startup before loading any assets.
///
/// @note  The singleton instance is accessed via AssetManager::instance().
////////////////////////////////////////////////////////////////////////////////////////////////////
class AssetManager
{
public:

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Oversampling factor applied to all font point sizes at load time.
    ///        A font requested at 12pt is rasterized at 48pt (12 * 4).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr float FONT_UPSCALING_FACTOR = 2.f;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Reciprocal of FONT_UPSCALING_FACTOR. Apply as a scale factor when rendering text
    ///        to restore the font to its intended display size.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr float INVERSE_FONT_UPSCALING_FACTOR = 1.f / FONT_UPSCALING_FACTOR;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The AssetManager singleton instance. Creates the instance on first call.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static AssetManager& instance();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Sets the root directory from which all asset paths are resolved.
    ///
    /// @note  Must be called before any load_font() call. Typically called once at engine startup
    ///        with the path to the deployed data directory.
    ///
    /// @param [in] path - Absolute or relative path to the data root directory.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static void set_data_root(std::string_view path);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief AssetManager destructor. Releases all remaining font resources.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ~AssetManager();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Releases all loaded font resources regardless of external reference count.
    ///
    /// @warning Any std::weak_ptr<TTF_Font> previously returned by get_font() will expire after
    ///          this call. Callers must re-load fonts before rendering.
    ///
    /// @return @c true if all resources were freed successfully.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool free_all_resources();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Loads a font at the specified point size (scaled by FONT_UPSCALING_FACTOR).
    ///        If the font is already loaded at that size, this is a no-op.
    ///
    /// @param [in] path       - Font file path relative to the data root.
    /// @param [in] point_size - Desired display size in points (before upscaling).
    ///
    /// @return @c true if the font was loaded (or was already present), @c false on error.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool load_font(std::string_view path, int point_size);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Returns a weak reference to the font loaded at the given path and point size.
    ///        Calls load_font() automatically if the font has not yet been loaded.
    ///
    /// @param [in] path       - Font file path relative to the data root.
    /// @param [in] point_size - Desired display size in points (before upscaling).
    ///
    /// @return A @c std::weak_ptr<TTF_Font>. Lock before use; expires if the font is freed.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::weak_ptr<TTF_Font> get_font(std::string_view path, int point_size);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Frees all point sizes of the font at the given path.
    ///
    /// @param [in] path - Font file path relative to the data root.
    ///
    /// @return @c true if any fonts were freed.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool free_font(std::string_view path);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Frees the specific point size of the font at the given path.
    ///
    /// @param [in] path       - Font file path relative to the data root.
    /// @param [in] point_size - The point size to free (before upscaling).
    ///
    /// @return @c true if the font was found and freed.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool free_font(std::string_view path, int point_size);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Frees all loaded fonts.
    ///
    /// @return @c true if any fonts were freed.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool free_fonts();

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Private constructor.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    AssetManager();

    /// @brief Key type for the font store: (relative path, point size) pair.
    using FontPair = std::pair<std::string, int>;

    /// @brief Absolute path to the data root directory.
    static std::string s_data_root;

    /// @brief Font store mapping (path, point_size) keys to shared TTF_Font handles.
    std::map<FontPair, std::shared_ptr<TTF_Font>> m_font_store{};
};

} // namespace Core

#endif // CORE_ASSET_MANAGER_H
