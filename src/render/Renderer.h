#ifndef _XENODON_RENDER_RENDERER_H
#define _XENODON_RENDER_RENDERER_H

#include <vector>
#include <vulkan/vulkan.hpp>
#include "graphics/Device.h"
#include "present/Display.h"

class Renderer {
    Display* display;

public:
    Renderer(Display* display);
};

#endif
