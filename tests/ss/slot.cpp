#include <catch2/catch.hpp>

#include "ss/signal.hpp"
#include "ss/slot.hpp"

TEST_CASE("sigslot")
{
    SECTION("slot")
    {
        int i = 0;

        ss::slot<void(), 8> s0([&i](auto* self) {
            (void) self;
            i = 5;
        });

        ss::slot<void(), 8> s1 = std::move(s0);

        static_assert(std::is_constructible_v<ss::slot<void(), 8>,
                                              ss::slot<void(), 4>&&>);
    }
    SECTION("signal")
    {
        int i = 0;

        ss::signal<void(int&)> sig;

        SECTION("slot_connect")
        {
            ss::slot<void(int&)> s0{[](auto&& self, int& i) {
                (void) self;
                ++i;
            }};

            sig.connect(s0);

            REQUIRE(i == 0);
            sig(i);
            REQUIRE(i == 1);

            auto s1 = sig.connect([](auto& i) { ++i; });

            sig(i);
            REQUIRE(i == 3);

            s0.disconnect();

            sig(i);
            REQUIRE(i == 4);
        }

        // auto s0 = sig.connect([]())
    }
}