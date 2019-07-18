#pragma once

#include "wlr.hpp"

class server;

class keyboard
{
private:
    server*           server_;
    wlr_input_device* device_;

    wl_listener modifiers_;
    wl_listener key_;

public:
    keyboard(server* serv, wlr_input_device* device);

    static void handle_modifiers(wl_listener* listener, void*);

    static void handle_key(wl_listener* listener, void* data);
};