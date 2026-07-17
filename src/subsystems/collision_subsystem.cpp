// ////////////////////////////////////////////////////////////////////////////////////////////////////
// /// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
// ////////////////////////////////////////////////////////////////////////////////////////////////////
// #include "core/subsystems/collision_subsystem.h"

// #include "core/entity.h"
// #include "core/layer.h"
// #include "core/components/collision_component.h"

// namespace Core
// {

// //--------------------------------------------------------------------------------------------------
// CollisionSubsystem::CollisionSubsystem()
//     : m_layer_map()
// {
// }

// //--------------------------------------------------------------------------------------------------
// CollisionSubsystem::~CollisionSubsystem()
// {
// }

// //--------------------------------------------------------------------------------------------------
// void CollisionSubsystem::register_collider(std::weak_ptr<CollisionComponent> collision_component)
// {
//     if (auto component = collision_component.lock())
//     {
//         Layer& layer = component->get_owner().get_owning_layer();
//         std::weak_ptr<Layer> layer_key = layer.weak_from_this();
//         if (m_layer_map.contains(layer_key))
//         {
//             m_layer_map[layer_key].push_back(collision_component);
//         }
//     }
// }

// //--------------------------------------------------------------------------------------------------
// void CollisionSubsystem::unregister_collider(std::weak_ptr<CollisionComponent> collision_component)
// {
//     if (auto component = collision_component.lock())
//     {
//         Layer& layer = component->get_owner().get_owning_layer();
//         std::weak_ptr<Layer> layer_key = layer.weak_from_this();
//         if (m_layer_map.contains(layer_key))
//         {
//             auto begin = m_layer_map[layer_key].begin();
//             auto end = m_layer_map[layer_key].end();
//             auto it = std::find_if(
//                 begin,
//                 end,
//                 [&](std::weak_ptr<CollisionComponent> const& other)
//                 {
//                     return !other.owner_before(collision_component) &&
//                            !collision_component.owner_before(other);
//                 });

//             if (it != end)
//             {
//                 m_layer_map[layer_key].erase(it);
//             }
//         }
//     }

//     std::erase_if(m_layer_map, [](auto const& pair)
//     {
//        return pair.first.expired();
//     });
// }

// //--------------------------------------------------------------------------------------------------
// std::vector<std::weak_ptr<CollisionComponent>> const&
// CollisionSubsystem::get_all_colliders_on_my_layer(CollisionComponent& context) const
// {
//     Layer& layer = context.get_owner().get_owning_layer();
//     std::weak_ptr<Layer> layer_key = layer.weak_from_this();
//     if (m_layer_map.contains(layer_key))
//     {
//         return m_layer_map.at(layer_key);
//     }

//     static const std::vector<std::weak_ptr<CollisionComponent>> empty{};
//     return empty;
// }

// } // namespace Core
