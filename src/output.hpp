#pragma once

#include "wl/listener.hpp"
#include "wlr.hpp"

class server;

class output
{
private:
    server*      server_;
    wlr_output*  wlr_output_;
    wl::listener frame_;

public:
    output(server* serv, wlr_output* output);
};