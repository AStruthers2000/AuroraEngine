////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/asset_manager.h"

namespace Core
{

std::string AssetManager::s_data_root = "aurora-engine/";

//--------------------------------------------------------------------------------------------------
AssetManager::AssetManager()
{
    if (!TTF_Init())
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Error",
                                 "Failed to initialize SDL3 TTF!",
                                 nullptr);
        std::exit(-1);
    }
}

//--------------------------------------------------------------------------------------------------
AssetManager::~AssetManager()
{
    free_all_resources();
}

//--------------------------------------------------------------------------------------------------
AssetManager& AssetManager::instance()
{
    static AssetManager inst;
    return inst;
}

//--------------------------------------------------------------------------------------------------
void AssetManager::set_data_root(std::string_view path)
{
    s_data_root = path;
    // Ensure the root ends with a path separator
    if (!s_data_root.empty() && s_data_root.back() != '/' && s_data_root.back() != '\\')
    {
        s_data_root += '/';
    }
}

//--------------------------------------------------------------------------------------------------
bool AssetManager::free_all_resources()
{
    bool const fonts_freed = free_fonts();

    return fonts_freed;
}

//--------------------------------------------------------------------------------------------------
bool AssetManager::load_font(std::string_view path, int point_size)
{
    std::string full_path = s_data_root + std::string(path);
    TTF_Font* font = TTF_OpenFont(full_path.c_str(), point_size * FONT_UPSCALING_FACTOR);
    if (font == nullptr)
    {
        SDL_Log("Failed to load font %s in size %d", full_path.c_str(), point_size);
        return false;
    }

    // Emplace a (path, size) pair as the key, with a shared_ptr<TTF_Font> as the value. TTF_Font
    // has a special destructor/free operator, TTF_CloseFont(), so the shared_ptr is constructed
    // with this custom deleter.
    m_font_store.try_emplace(
        std::make_pair(std::string(path), point_size),
        std::shared_ptr<TTF_Font>(font, TTF_CloseFont));

    return true;
}

//--------------------------------------------------------------------------------------------------
std::weak_ptr<TTF_Font> AssetManager::get_font(std::string_view path, int point_size)
{
    std::weak_ptr<TTF_Font> font{};

    std::pair<std::string, int> font_key{ std::string(path), point_size };

    // If the font isn't already in the store, try and load it
    if (!m_font_store.contains(font_key))
    {
        load_font(path, point_size);
    }
    
    // Try to find the font in the font store (either it was already loaded, or it wasn't loaded but
    // now is loaded)
    if (m_font_store.contains(font_key))
    {
        font = m_font_store[font_key];
    }

    return font;
}

//--------------------------------------------------------------------------------------------------
bool AssetManager::free_font(std::string_view path)
{
    auto removed = std::erase_if(m_font_store, [path](auto const& entry)
    {
        auto const& key = entry.first;
        std::string_view const key_path = key.first;
        return key_path == path;
    });

    return removed > 0;
}

//--------------------------------------------------------------------------------------------------
bool AssetManager::free_font(std::string_view path, int point_size)
{
    std::pair<std::string, int> font_key{ std::string(path), point_size };

    auto const elements_removed = m_font_store.erase(font_key);
    return elements_removed > 0;
}

//--------------------------------------------------------------------------------------------------
bool AssetManager::free_fonts()
{
    m_font_store.clear();
    return true;
}

} // namespace Core
