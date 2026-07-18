#include "overlap_volume.h"
#include "better_game_object.h"
#include "game_mode.h"

void OverlapVolume::initialize()
{
    my_volume =
    {
        .x = get_transform().get_position().x,
        .y = get_transform().get_position().y,
        .w = get_transform().get_scale().x,
        .h = get_transform().get_scale().y,
    };
}

void OverlapVolume::update(float delta_time)
{
    for (auto const& [registrant, response] : m_registered_game_objects)
    {
        if (auto registrant_ptr = registrant.lock())
        {
            if (check_for_overlap(registrant_ptr.get()))
            {
                response(this);
                if (m_overlap_side == EOverlapPosition::LeftSide)
                {
                    m_game_mode.score_goal(2);
                }
                else if (m_overlap_side == EOverlapPosition::RightSide)
                {
                    m_game_mode.score_goal(1);
                }
            }
        }
    }

    std::erase_if(m_registered_game_objects, [](auto const& pair)
    {
       return pair.first.expired();
    });
}

void OverlapVolume::render(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderRect(renderer, &my_volume);
}

void OverlapVolume::cleanup()
{

}

void OverlapVolume::register_collision_response(const std::weak_ptr<BetterGameObject>& registrant, OverlapCallback &&overlap_response)
{
    m_registered_game_objects.try_emplace(registrant, std::move(overlap_response));
}

bool OverlapVolume::check_for_overlap(BetterGameObject const* other)
{
    SDL_FRect other_volume = other->get_collider();
    SDL_FRect overlap;

    bool has_overlap{ false };

    if (SDL_GetRectIntersectionFloat(&my_volume, &other_volume, &overlap))
    {
        constexpr float tolerance = 0.001f;
        if (abs(overlap.w) > tolerance && abs(overlap.h) > tolerance)
        {
            has_overlap = true;
        }
    }

    return has_overlap;
}
