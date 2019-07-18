#include "server.hpp"

#include <algorithm>

#include "keyboard.hpp"
#include "output.hpp"
#include "view.hpp"

server::server(wl_display* dpy)
    : display_{dpy}, backend_{wlr_backend_autocreate(display_, nullptr)},
      renderer_{wlr_backend_get_renderer(backend_)},

      xdg_shell_{wlr_xdg_shell_create(display_)}, cursor_{wlr_cursor_create()},
      cursor_mgr_{wlr_xcursor_manager_create(nullptr, 24)},
      seat_{wlr_seat_create(display_, "seat0")}, output_layout_{
                                                     wlr_output_layout_create()}
{
    wlr_renderer_init_wl_display(renderer_, display_);

    wlr_compositor_create(display_, renderer_);
    wlr_data_device_manager_create(display_);

    new_output_.notify = handle_new_output;
    wl_signal_add(&backend_->events.new_output, &new_output_);

    new_xdg_surface_.notify = handle_new_xdg_surface;
    wl_signal_add(&xdg_shell_->events.new_surface, &new_xdg_surface_);

    xdg_surface_destroy_.notify = handle_xdg_surface_destroy;

    wlr_cursor_attach_output_layout(cursor_, output_layout_);
    wlr_xcursor_manager_load(cursor_mgr_, 1);

    cursor_motion_.notify = handle_cursor_motion;
    wl_signal_add(&cursor_->events.motion, &cursor_motion_);
    cursor_motion_abs_.notify = handle_cursor_motion_absolute;
    wl_signal_add(&cursor_->events.motion_absolute, &cursor_motion_abs_);
    cursor_button_.notify = handle_cursor_button;
    wl_signal_add(&cursor_->events.button, &cursor_button_);
    cursor_axis_.notify = handle_cursor_axis;
    wl_signal_add(&cursor_->events.axis, &cursor_axis_);
    cursor_frame_.notify = handle_cursor_frame;
    wl_signal_add(&cursor_->events.frame, &cursor_frame_);

    new_input_.notify = handle_new_input;
    wl_signal_add(&backend_->events.new_input, &new_input_);

    request_cursor_.notify = handle_request_cursor;
    wl_signal_add(&seat_->events.request_set_cursor, &request_cursor_);

    const char* socket = wl_display_add_socket_auto(display_);
    if (!socket)
    {
        throw std::runtime_error{"failed to add socket for display"};
    }

    if (!wlr_backend_start(backend_))
    {
        throw std::runtime_error{"failed to start backend"};
    }

    setenv("WAYLAND_DISPLAY", socket, true);

    wlr_log(WLR_INFO, "Running Trinkster on WAYLAND_DISPLAY=%s", socket);
}

void server::run()
{
    wl_display_run(display_);
}

void server::add_keyboard(wlr_input_device* device)
{
    auto* kb = new keyboard{this, device};
    keyboards_.push_back(kb);
}

void server::add_pointer(wlr_input_device* device)
{
    wlr_cursor_attach_input_device(cursor_, device);
}

std::optional<std::tuple<view*, wlr_surface*, glm::dvec2>>
server::view_at(double lx, double ly)
{
    for (auto&& view : views_)
    {
        auto surface_at_res = view->surface_at(lx, ly);

        if (surface_at_res)
        {
            auto [surf, pos] = *surface_at_res;
            return std::make_tuple(view.get(), surf, pos);
        }
    }

    return std::nullopt;
}

void server::process_cursor_move(uint32_t time)
{
    (void) time;
    grabbed_view_->x = cursor_->x - grab_x_;
    grabbed_view_->y = cursor_->y - grab_y_;
}

void server::process_cursor_resize(uint32_t time)
{
    (void) time;
    // TODO wait for prepared buffer and then commit
    // this will be handled in the view::set_size
    auto* view = grabbed_view_;

    double dx = cursor_->x - grab_x_;
    double dy = cursor_->y - grab_y_;

    double x = view->x;
    double y = view->y;

    uint32_t width  = grab_width_;
    uint32_t height = grab_height_;

    if (resize_edges_ & WLR_EDGE_TOP)
    {
        y = grab_y_ + dy;
        height -= dy;
        if (height < 1)
        {
            y += height;
        }
    }
    else if (resize_edges_ & WLR_EDGE_BOTTOM)
    {
        height += dy;
    }

    if (resize_edges_ & WLR_EDGE_LEFT)
    {
        x = grab_x_ + dx;
        width -= dx;
        if (width < 1)
        {
            x += width;
        }
    }
    else if (resize_edges_ & WLR_EDGE_RIGHT)
    {
        width += dx;
    }

    view->x = x;
    view->y = y;

    view->set_size(width, height);
}

void server::process_cursor_motion(uint32_t time)
{
    if (cursor_mode_ == cursor_mode::move)
    {
        process_cursor_move(time);
        return;
    }
    else if (cursor_mode_ == cursor_mode::resize)
    {
        process_cursor_resize(time);
        return;
    }

    // we're in passthrough mode
    auto* seat     = seat_;
    auto  view_opt = view_at(cursor_->x, cursor_->y);

    if (!view_opt)
    {
        // reset cursor image because no view under cursor
        wlr_xcursor_manager_set_cursor_image(cursor_mgr_, "left_ptr", cursor_);
        wlr_seat_pointer_clear_focus(seat);
        return;
    }

    // destructure now
    auto [view, surf, pos] = *view_opt;
    (void) view;

    bool focus_changed = seat->pointer_state.focused_surface != surf;

    if (focus_changed)
    {
        wlr_seat_pointer_notify_enter(seat, surf, pos.x, pos.y);
    }
    else
    {
        wlr_seat_pointer_notify_motion(seat, time, pos.x, pos.y);
    }
}

void server::handle_new_input(wl_listener* listener, void* data)
{
    server* self   = wl_container_of(listener, self, new_input_);
    auto*   device = static_cast<wlr_input_device*>(data);

    switch (device->type)
    {
    case WLR_INPUT_DEVICE_KEYBOARD:
        self->add_keyboard(device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        self->add_pointer(device);
        break;
    case WLR_INPUT_DEVICE_SWITCH:
    case WLR_INPUT_DEVICE_TABLET_PAD:
    case WLR_INPUT_DEVICE_TABLET_TOOL:
    case WLR_INPUT_DEVICE_TOUCH:
        break;
    }

    // set our new capabilities
    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!std::empty(self->keyboards_))
    {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }

    wlr_seat_set_capabilities(self->seat(), caps);
}

void server::handle_request_cursor(wl_listener* listener, void* data)
{
    server* self = wl_container_of(listener, self, request_cursor_);

    auto* event = static_cast<wlr_seat_pointer_request_set_cursor_event*>(data);
    auto* focused_client = self->seat()->pointer_state.focused_client;

    if (focused_client == event->seat_client)
    {
        wlr_cursor_set_surface(
            self->cursor_, event->surface, event->hotspot_x, event->hotspot_y);
    }
    else
    {
        pid_t pid;
        uid_t uid;
        gid_t gid;
        wl_client_get_credentials(event->seat_client->client, &pid, &uid, &gid);

        wlr_log(WLR_INFO,
                "pid: \"%ld\" sent bad cursor request",
                static_cast<uint64_t>(pid));
    }
}

void server::handle_cursor_motion(wl_listener* listener, void* data)
{
    server* self  = wl_container_of(listener, self, cursor_motion_);
    auto*   event = static_cast<wlr_event_pointer_motion*>(data);

    wlr_cursor_move(
        self->cursor_, event->device, event->delta_x, event->delta_y);
    self->process_cursor_motion(event->time_msec);
}

void server::handle_cursor_motion_absolute(wl_listener* listener, void* data)
{
    server* self  = wl_container_of(listener, self, cursor_motion_abs_);
    auto*   event = static_cast<wlr_event_pointer_motion_absolute*>(data);

    wlr_cursor_warp_absolute(self->cursor_, event->device, event->x, event->y);
    self->process_cursor_motion(event->time_msec);
}

void server::handle_cursor_button(wl_listener* listener, void* data)
{
    server* self  = wl_container_of(listener, self, cursor_button_);
    auto*   event = static_cast<wlr_event_pointer_button*>(data);

    wlr_seat_pointer_notify_button(
        self->seat(), event->time_msec, event->button, event->state);

    auto view_opt = self->view_at(self->cursor_->x, self->cursor_->y);

    if (event->state == WLR_BUTTON_RELEASED)
    {
        self->cursor_mode_ = cursor_mode::passthrough;
    }
    else
    {
        if (view_opt)
        {
            auto [view, surf, pos] = *view_opt;
            (void) pos;
            view->keyboard_focus(*surf);
        }
    }
}

void server::handle_cursor_axis(wl_listener* listener, void* data)
{
    server* self  = wl_container_of(listener, self, cursor_axis_);
    auto*   event = static_cast<wlr_event_pointer_axis*>(data);

    wlr_seat_pointer_notify_axis(self->seat(),
                                 event->time_msec,
                                 event->orientation,
                                 event->delta,
                                 event->delta_discrete,
                                 event->source);
}

void server::handle_cursor_frame(wl_listener* listener, void* data)
{
    (void) data;
    server* self = wl_container_of(listener, self, cursor_frame_);

    wlr_seat_pointer_notify_frame(self->seat());
}

void server::handle_new_output(wl_listener* listener, void* data)
{
    server* self       = wl_container_of(listener, self, new_output_);
    auto*   wlr_output = static_cast<struct wlr_output*>(data);

    // just pick the first mode for now
    if (!wl_list_empty(&wlr_output->modes))
    {
        wlr_output_mode* mode =
            wl_container_of(wlr_output->modes.prev, mode, link);
        wlr_output_set_mode(wlr_output, mode);
    }

    auto* out = new output{self, wlr_output};

    self->outputs_.push_back(out);
    wlr_output_layout_add_auto(self->output_layout(), wlr_output);
    wlr_output_create_global(wlr_output);
}

void server::handle_new_xdg_surface(wl_listener* listener, void* data)
{
    server* self        = wl_container_of(listener, self, new_xdg_surface_);
    auto*   xdg_surface = static_cast<wlr_xdg_surface*>(data);

    if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL)
    {
        // don't care about popups
        return;
    }

    // this is a top level let's create a view for it
    auto v = std::make_unique<view>(self, xdg_surface);
    wl_signal_add(&v->xdg_surface()->events.destroy,
                  &self->xdg_surface_destroy_);
    self->views_.push_back(std::move(v));
}

void server::handle_xdg_surface_destroy(wl_listener* listener, void* data)
{
    server* self        = wl_container_of(listener, self, xdg_surface_destroy_);
    auto*   xdg_surface = static_cast<wlr_xdg_surface*>(data);

    auto& views = self->views_;

    auto it = std::find_if(std::begin(views), std::end(views), [&](auto&& v) {
        return v->xdg_surface() == xdg_surface;
    });

    views.erase(it);
}