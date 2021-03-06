#ifndef _XENODON_MAIN_LOOP_H
#define _XENODON_MAIN_LOOP_H

#include <string_view>
#include <filesystem>
#include <cstddef>
#include "utility/Span.h"
#include "math/Vec.h"

struct EventDispatcher;
struct Display;

struct RenderParameters {
    std::filesystem::path volume_path;
    std::string_view volume_type_override;
    std::string_view shader;
    std::filesystem::path stats_save_path;
    Vec3F voxel_ratio = Vec3F(1, 1, 1);
    std::string_view camera;
    float emission_coeff = 1.f;
    size_t repeat = 1;
};

void main_loop(EventDispatcher& dispatcher, Display* display, const RenderParameters& render_params);

#endif
