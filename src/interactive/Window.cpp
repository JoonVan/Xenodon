#include "interactive/Window.h"
#include <string_view>
#include <cstdint>
#include <iostream>

namespace {
    using std::literals::operator ""sv;
    using std::literals::operator ""s;

    constexpr const uint32_t EVENT_MASK =
          XCB_EVENT_MASK_KEY_RELEASE
        | XCB_EVENT_MASK_KEY_PRESS
        | XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_STRUCTURE_NOTIFY
        | XCB_EVENT_MASK_POINTER_MOTION
        | XCB_EVENT_MASK_BUTTON_PRESS
        | XCB_EVENT_MASK_BUTTON_RELEASE;
}

WindowContext::WindowContext(xcb_connection_t* connection):
    connection(connection),
    atom_wm_delete_window(this->atom(false, std::string_view{"WM_DELETE_WINDOW"})) {
}

AtomReply WindowContext::atom(bool only_if_exists, const std::string_view& str) const {
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(
        this->connection,
        only_if_exists,
        static_cast<uint16_t>(str.size()),
        str.data()
    );

    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(this->connection, cookie, nullptr);
    return AtomReply(reply);
}

Window::Window(WindowContext& window_context, xcb_screen_t* screen):
    connection(window_context.connection),
    screen(screen),
    xid(xcb_generate_id(window_context.connection)) {

    // Enable sending delete events if the window is closed.
    AtomReply atom_wm_protocols = window_context.atom(true, "WM_PROTOCOLS"sv);

    xcb_change_property(
        this->connection,
        XCB_PROP_MODE_REPLACE,
        this->xid,
        atom_wm_protocols->atom,
        XCB_ATOM_ATOM,
        32,
        1,
        &window_context.atom_wm_delete_window->atom
    );
}

Window::Window(WindowContext& window_context, xcb_screen_t* screen, Mode::Fullscreen):
    Window(window_context, screen) {

    uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
    uint32_t value_list[] = {
        this->screen->black_pixel,
        true,
        EVENT_MASK
    };

    xcb_create_window_checked(
        this->connection,
        XCB_COPY_FROM_PARENT,
        this->xid,
        screen->root,
        0,
        0,
        screen->width_in_pixels,
        screen->height_in_pixels,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        value_mask,
        value_list
    );

    xcb_map_window(this->connection, this->xid);
    xcb_flush(this->connection);

    xcb_grab_keyboard(
        this->connection,
        1,
        this->xid,
        XCB_CURRENT_TIME,
        XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC
    );

    xcb_grab_pointer(
        this->connection,
        1,
        this->xid,
        XCB_NONE,
        XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC,
        this->xid,
        XCB_NONE,
        XCB_CURRENT_TIME
    );
}

Window::Window(WindowContext& window_context, xcb_screen_t* screen, Mode::Windowed, uint16_t width, uint16_t height):
    Window(window_context, screen) {

    uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t value_list[] = {
        this->screen->black_pixel,
        EVENT_MASK
    };

    xcb_create_window_checked(
        this->connection,
        XCB_COPY_FROM_PARENT,
        this->xid,
        screen->root,
        0,
        0,
        width,
        height,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        value_mask,
        value_list
    );

    xcb_map_window(this->connection, this->xid);
    xcb_flush(this->connection);
}

Window::Window(Window&& other):
    connection(other.connection),
    screen(other.screen),
    xid(other.xid) {
    other.connection = nullptr;
}

Window& Window::operator=(Window&& other) {
    std::swap(this->connection, other.connection);
    std::swap(this->screen, other.screen);
    std::swap(this->xid, other.xid);
    return *this;
}

Window::~Window() {
    if (this->connection)
        xcb_destroy_window(this->connection, this->xid);
}

vk::XcbSurfaceCreateInfoKHR Window::surface_create_info() const {
    return vk::XcbSurfaceCreateInfoKHR(
        {},
        this->connection,
        this->xid
    );
}

vk::Rect2D Window::geometry() const {
    xcb_get_geometry_cookie_t cookie = xcb_get_geometry(this->connection, this->xid);
    auto reply = MallocPtr<xcb_get_geometry_reply_t>(xcb_get_geometry_reply(this->connection, cookie, nullptr));
    return vk::Rect2D(
        {
            static_cast<int32_t>(reply->x),
            static_cast<int32_t>(reply->y)
        },
        {
            static_cast<uint32_t>(reply->width),
            static_cast<uint32_t>(reply->height)
        }
    );
}
