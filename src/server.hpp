#pragma once

#include "cursor.hpp"
#include "wl/listener.hpp"
#include "wlr.hpp"

#include <memory>
#include <optional>
#include <tuple>
#include <vector>

#include <glm/vec2.hpp>
#include <ws/ws.hpp>

class keyboard;
class output;
class view;

class server
{
private:
    wl_display*   display_;
    wlr_backend*  backend_;
    wlr_renderer* renderer_;

    wlr_xdg_shell*                   xdg_shell_;
    ws::slot<void(wlr_xdg_surface&)> new_xdg_surface_;
    // wl::listener                       new_xdg_surface_;
    ws::slot<void(wlr_xdg_surface&)> xdg_surface_destroy_;
    // wl::listener                       xdg_surface_destroy_;
    std::vector<std::unique_ptr<view>> views_;

    wlr_cursor*          cursor_;
    wlr_xcursor_manager* cursor_mgr_;
    wl_listener          cursor_motion_;
    wl_listener          cursor_motion_abs_;
    wl_listener          cursor_button_;
    wl_listener          cursor_axis_;
    wl_listener          cursor_frame_;

    wlr_seat*              seat_;
    wl_listener            new_input_;
    wl_listener            request_cursor_;
    std::vector<keyboard*> keyboards_;
    cursor_mode            cursor_mode_;

    // TODO move these out of here, into active_event or similar
    view*    grabbed_view_;
    double   grab_x_, grab_y_;
    uint32_t grab_width_, grab_height_;
    uint32_t resize_edges_;

    wlr_output_layout*   output_layout_;
    std::vector<output*> outputs_;
    wl_listener          new_output_;

public:
    server(wl_display* dpy);

    void run();

    wlr_seat* seat() noexcept
    {
        return seat_;
    }

    auto& views()
    {
        return views_;
    }

    wlr_output_layout* output_layout()
    {
        return output_layout_;
    }

    wlr_cursor* cursor()
    {
        return cursor_;
    }

    wlr_renderer* renderer()
    {
        return renderer_;
    }

    void set_grab_x(double x)
    {
        grab_x_ = x;
    }

    void set_grab_y(double y)
    {
        grab_y_ = y;
    }

    void set_grab_width(uint32_t width)
    {
        grab_width_ = width;
    }

    void set_grab_height(uint32_t height)
    {
        grab_height_ = height;
    }

    void set_resize_edges(uint32_t edges)
    {
        resize_edges_ = edges;
    }

    void add_keyboard(wlr_input_device* device);
    void add_pointer(wlr_input_device* device);

    std::optional<std::tuple<view*, wlr_surface*, glm::dvec2>>
    view_at(double lx, double ly);

    void process_cursor_move(uint32_t time);
    void process_cursor_resize(uint32_t time);
    void process_cursor_motion(uint32_t time);

    static void handle_new_input(wl_listener* listener, void* data);
    static void handle_request_cursor(wl_listener* listener, void* data);
    static void handle_cursor_motion(wl_listener* listener, void* data);
    static void handle_cursor_motion_absolute(wl_listener* listener,
                                              void*        data);
    static void handle_cursor_button(wl_listener* listener, void* data);
    static void handle_cursor_axis(wl_listener* listener, void* data);
    static void handle_cursor_frame(wl_listener* listener, void* data);

    static void handle_new_output(wl_listener* listener, void* data);
};