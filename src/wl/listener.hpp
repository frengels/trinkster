#pragma once

#include <wayland-server-core.h>

namespace wl
{
class listener : public wl_listener
{
public:
    friend void connect(wl_signal&, wl::listener&) noexcept;

public:
    listener(wl_notify_func_t fn) noexcept : wl_listener{{}, fn}
    {}

    listener(const listener&) = delete;
    listener& operator=(const listener&) = delete;

    listener(listener&& other) noexcept;
    listener& operator=(listener&& other) noexcept;

    void remove() noexcept;

    void swap(listener& other) noexcept;
};

void connect(wl_signal&, wl_listener&) noexcept;
void connect(wl_signal&, wl::listener&) noexcept;

} // namespace wl