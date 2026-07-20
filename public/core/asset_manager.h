////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
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
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class AssetManager
{
public:
    static constexpr float FONT_UPSCALING_FACTOR = 4.f;
    static constexpr float INVERSE_FONT_UPSCALING_FACTOR = 1.f / FONT_UPSCALING_FACTOR;

    static AssetManager& instance();
    static void set_data_root(std::string_view path);
    ~AssetManager();

    bool free_all_resources();

    bool load_font(std::string_view path, int point_size);
    std::weak_ptr<TTF_Font> get_font(std::string_view path, int point_size);
    bool free_font(std::string_view path);
    bool free_font(std::string_view path, int point_size);
    bool free_fonts();

private:
    AssetManager();

    static std::string s_data_root;

    using FontPair = std::pair<std::string, int>;
    std::map<FontPair, std::shared_ptr<TTF_Font>> m_font_store{};
};

} // namespace Core

#endif // CORE_ASSET_MANAGER_H
