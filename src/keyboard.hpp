#pragma once

#include "wl/listener.hpp"
#include "wlr.hpp"

class server;

class keyboard
{
private:
    server*           server_;
    wlr_input_device* device_;

    wl::listener modifiers_;
    wl::listener key_;

public:
    keyboard(server* serv, wlr_input_device* device);
};