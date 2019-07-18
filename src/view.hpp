#pragma once

#include <optional>
#include <tuple>

#include <glm/vec2.hpp>

#include "cursor.hpp"
#include "wl/listener.hpp"
#include "wlr.hpp"

class server;

class view
{
private:
    server*          server_;
    wlr_xdg_surface* xdg_surface_;

    wl::listener map_;
    wl::listener unmap_;
    wl::listener request_move_;
    wl::listener request_resize_;

    bool mapped_;

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

    /// given 2 coordinates in layout space
    /// if present return the wlr_surface and surface local coordinates of the
    /// position within the surface.
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
};