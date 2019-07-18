#pragma once

#include "wlr.hpp"

class server;

class output
{
private:
    server*     server_;
    wlr_output* wlr_output_;
    wl_listener frame_;

public:
    output(server* serv, wlr_output* output);

    static void handle_frame(wl_listener* listener, void* data);
};