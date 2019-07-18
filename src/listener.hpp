#pragma once

#include <utility>

#include <wayland-server.h>

namespace wl
{
template<typename F>
class listener;

namespace detail
{
template<typename F>
struct data_argument;

template<typename Listener, typename T>
struct data_argument<void (*)(Listener, T)>
{
    using type = T;
};
} // namespace detail

template<typename F>
class listener
{
    using data_type = typename detail::data_argument<F>::type;

    static_assert(std::is_convertible<void*, data_type>::value,
                  "Cannot convert from void* to your 2nd argument type");

private:
    wl_listener             listener_;
    [[no_unique_address]] F fn_;

    static constexpr auto call_fn = [](wl_listener* _listener, void* _data) {
        data_type data   = static_cast<data_type>(_data);
        listener* listen = reinterpret_cast<listener*>(_listener);

        listen->fn_(listen, data);
    };

public:
    constexpr listener(F fn) : listener_{{}, call_fn}, fn_{std::move(fn)}
    {}

    constexpr listener(const listener&) = delete;
    constexpr listener& operator=(const listener&) = delete;

    // TODO make movable
    constexpr listener(listener&&) = delete;
    constexpr listener& operator=(listener&&) = delete;
};
} // namespace wl