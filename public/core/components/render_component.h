////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_COMPONENTS_RENDER_COMPONENT_H
#define CORE_COMPONENTS_RENDER_COMPONENT_H

#include "core/component.h"

namespace Core
{

class RenderComponent : public Component
{
public:
    struct Configuration : public Component::Configuration
    {

    };

    explicit RenderComponent(Entity& owner, Configuration const& config = {});
};

} // namespace Core

#endif // CORE_COMPONENTS_RENDER_COMPONENT_H