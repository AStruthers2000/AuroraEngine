////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Convenience single-include header for Aurora Engine game code.
///
/// Include this one header to access the full public API: Entity, Component, Layer, Engine,
/// AssetManager, the built-in Component and Entity types, the event system, and SDL3.
///
/// Game projects should include this header rather than individual engine headers.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef AURORA_ENGINE_PUBLIC_H
#define AURORA_ENGINE_PUBLIC_H

#include "core/asset_manager.h"
#include "core/component.h"
#include "core/engine.h"
#include "core/entity.h"
#include "core/layer.h"
#include "core/sdl_color_ops.h"

#include "core/components/rect_render_component.h"
#include "core/components/text_render_component.h"
#include "core/components/transform_component.h"
#include "core/components/anchor_component.h"

#include "core/entities/ui/button.h"
#include "core/entities/ui/label.h"
#include "core/entities/ui/panel.h"

#include "core/events/events_public.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#endif // AURORA_ENGINE_PUBLIC_H
