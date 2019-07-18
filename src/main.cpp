#include <vector>

#include <wayland-client.h>
#include <wayland-server.h>

#include "keyboard.hpp"
#include "server.hpp"
#include "view.hpp"
#include "wlr.hpp"

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    wlr_log_init(WLR_DEBUG, nullptr);

    ::server server{wl_display_create()};

    server.run();
}
