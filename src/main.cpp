#include <wayland-client.h>
#include <wayland-server.h>

#include "wlr.hpp"

class server
{
private:
    wl_display*   display_;
    wlr_backend*  backend_;
    wlr_renderer* renderer_;

    wlr_xdg_shell* xdg_shell_;
    wl_listener    new_xdg_surface_;
    wl_list        views_;

    wlr_cursor*          cursor_;
    wlr_xcursor_manager* cursor_mgr_;
    wl_listener          cursor_motion_;
    wl_listener          cursor_motion_abs_;
    wl_listener          cursor_button_;
    wl_listener          cursor_axis_;
    wl_listener          cursor_frame_;

    wlr_seat*   seat_;
    wl_listener new_input_;
    wl_listener request_cursor_;
    wl_list     keyboards_;
};

int main(int argc, char** argv)
{}
