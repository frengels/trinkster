#pragma once

#include <optional>
#include <tuple>

#include <glm/vec2.hpp>

#include "cursor.hpp"
#include "wlr.hpp"

class server;

class view
{
private:
    server*          server_;
    wlr_xdg_surface* xdg_surface_;
    wl_listener      map_;
    wl_listener      unmap_;
    wl_listener      request_move_;
    wl_listener      request_resize_;
    bool             mapped_;

public:
    int x, y;

public:
    view(server* serv, wlr_xdg_surface* surface);

    bool mapped() const
    {
        return mapped_;
    }

    wlr_xdg_surface* xdg_surface()
    {
        return xdg_surface_;
    }

    void keyboard_focus(wlr_surface& surf);

    std::optional<std::tuple<wlr_surface*, glm::dvec2>> surface_at(double lx,
                                                                   double ly);
    std::optional<std::tuple<wlr_surface*, glm::dvec2>>
    surface_at(const glm::dvec2& pos)
    {
        return surface_at(pos.x, pos.y);
    }

    void begin_interactive_move();
    void begin_interactive_resize(uint32_t edges);

    void set_size(uint32_t width, uint32_t height);

    static void handle_xdg_surface_map(wl_listener* listener, void* data);
    static void handle_xdg_surface_unmap(wl_listener* listener, void* data);
    static void handle_xdg_toplevel_request_move(wl_listener* listener,
                                                 void*        data);
    static void handle_xdg_toplevel_request_resize(wl_listener* listener,
                                                   void*        data);
};