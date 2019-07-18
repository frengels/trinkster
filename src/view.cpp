#include "view.hpp"

#include "server.hpp"

view::view(server* serv, wlr_xdg_surface* surface)
    : server_{serv}, xdg_surface_{surface}, map_{[](auto* listener, void*) {
          view* self    = wl_container_of(listener, self, map_);
          self->mapped_ = true;
          self->keyboard_focus(*self->xdg_surface()->surface);
      }},
      unmap_{[](auto* listener, void*) {
          view* self    = wl_container_of(listener, self, unmap_);
          self->mapped_ = false;
      }},
      request_move_{[](auto* listener, void* data) {
          // TODO check if it's a user requested move
          view* self  = wl_container_of(listener, self, request_move_);
          auto* event = static_cast<wlr_xdg_toplevel_move_event*>(data);
          (void) event;
          self->begin_interactive_move();
      }},
      request_resize_{[](auto* listener, void* data) {
          // TODO again check for user request
          view* self  = wl_container_of(listener, self, request_resize_);
          auto* event = static_cast<wlr_xdg_toplevel_resize_event*>(data);

          self->begin_interactive_resize(event->edges);
      }}
{
    wl::connect(xdg_surface_->events.map, map_);
    wl::connect(xdg_surface_->events.unmap, unmap_);

    auto* toplevel = xdg_surface_->toplevel;

    wl::connect(toplevel->events.request_move, request_move_);
    wl::connect(toplevel->events.request_resize, request_resize_);
}

void view::keyboard_focus(wlr_surface& surf)
{
    auto* server = server_;
    auto* seat   = server->seat();

    auto* prev_surface = seat->keyboard_state.focused_surface;

    if (prev_surface == std::addressof(surf))
    {
        return;
    }

    if (prev_surface)
    {
        auto* xdg_prev = wlr_xdg_surface_from_wlr_surface(prev_surface);
        wlr_xdg_toplevel_set_activated(xdg_prev, false);
    }

    auto* keyboard = wlr_seat_get_keyboard(seat);

    wlr_xdg_toplevel_set_activated(xdg_surface_, true);

    wlr_seat_keyboard_notify_enter(seat,
                                   xdg_surface_->surface,
                                   keyboard->keycodes,
                                   keyboard->num_keycodes,
                                   &keyboard->modifiers);
}

std::optional<std::tuple<wlr_surface*, glm::dvec2>> view::surface_at(double lx,
                                                                     double ly)
{
    double view_sx = lx - x;
    double view_sy = ly - y;

    double sx, sy;
    auto*  surface =
        wlr_xdg_surface_surface_at(xdg_surface_, view_sx, view_sy, &sx, &sy);

    if (surface)
    {
        return {std::make_tuple(surface, glm::dvec2{sx, sy})};
    }

    return std::nullopt;
}

void view::begin_interactive_move()
{
    auto* server = server_;

    server->set_grab_x(server->cursor()->x - x);
    server->set_grab_y(server->cursor()->y - y);
}

void view::begin_interactive_resize(uint32_t edges)
{
    auto* server = server_;

    wlr_box geo_box;
    wlr_xdg_surface_get_geometry(xdg_surface(), &geo_box);

    server->set_grab_x(server->cursor()->x + geo_box.x);
    server->set_grab_y(server->cursor()->y + geo_box.y);

    server->set_grab_width(geo_box.width);
    server->set_grab_height(geo_box.height);
    server->set_resize_edges(edges);
}

void view::set_size(uint32_t width, uint32_t height)
{
    // TODO: some more advanced buffer commit stuff
    wlr_xdg_toplevel_set_size(xdg_surface_, width, height);
}