#include <catch2/catch.hpp>

#include <iterator>

#include "ss/detail/intrusive_list.hpp"

struct intruded
{
    int i;

    ss::detail::link l_; // on purpose not in first spot

    constexpr intruded(int i) noexcept : i{i}
    {}
};

namespace ss
{
namespace detail
{
template<>
struct intrusive_link_offset<intruded>
    : std::integral_constant<std::size_t, offsetof(intruded, l_)>
{};
} // namespace detail
} // namespace ss

TEST_CASE("intrusive_list")
{
    ss::detail::intrusive_list<intruded> list;

    // check nothing crashes on empty list
    REQUIRE(list.empty());
    list.clear();

    intruded i0{0};
    intruded i1{1};
    intruded i2{2};
    intruded i3{3};

    SECTION("empty_list")
    {
        REQUIRE(!i0.l_);
        REQUIRE(!i1.l_);
        REQUIRE(!i2.l_);
        REQUIRE(!i3.l_);

        for (auto& i : list)
        {
            REQUIRE(i.i == 10000); // impossible
        }
    }

    list.push_back(i0);
    list.push_back(i1);
    list.push_back(i2);
    list.push_back(i3);

    REQUIRE(i0.l_);
    REQUIRE(i1.l_);
    REQUIRE(i2.l_);
    REQUIRE(i3.l_);

    SECTION("in_order")
    {
        int count = 0;

        for (auto& i : list)
        {
            REQUIRE(i.i == count++);
        }
    }

    SECTION("clear")
    {
        REQUIRE(!list.empty());

        list.clear();

        REQUIRE(list.empty());

        REQUIRE(!i0.l_);
        REQUIRE(!i1.l_);
        REQUIRE(!i2.l_);
        REQUIRE(!i3.l_);

        for (auto& i : list)
        {
            REQUIRE(i.i == 10000); // impossible again, demonstrating emptyness
        }
    }

    SECTION("swaps")
    {
        auto it0 = list.begin();
        auto it1 = std::next(it0);
        auto it2 = std::next(it1);
        auto it3 = std::next(it2);

        REQUIRE(it0->i == 0);
        REQUIRE(it1->i == 1);
        REQUIRE(it2->i == 2);
        REQUIRE(it3->i == 3);

        std::iter_swap(it0, it1);
        std::iter_swap(it2, it3);

        // iterators should be swapped
        REQUIRE(it0->i == 1);
        REQUIRE(it1->i == 0);
        REQUIRE(it2->i == 3);
        REQUIRE(it3->i == 2);

        // the original values will be changed because of op= behavior
    }
}