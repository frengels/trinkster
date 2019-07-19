#pragma once

#include <type_traits>

#include "ss/detail/intrusive_list.hpp"
#include "ss/slot.hpp"

namespace ss
{
template<typename Ret, typename... Args>
class signal;

template<typename Ret, typename... Args>
class signal<Ret(Args...)>
{
private:
    detail::intrusive_list<ss::slot_base<Ret(Args...)>> slots_;

public:
    template<typename... UArgs>
    std::enable_if_t<std::is_invocable_v<ss::slot_base<Ret(Args...)>, UArgs...>>
    operator()(UArgs&&... args) const
    {
        for (auto& slot : slots_)
        {
            slot(std::forward<UArgs>(args)...);
        }
    }

    constexpr void connect(ss::slot_base<Ret(Args...)>& s) noexcept
    {
        slots_.push_back(s);
    }

    template<typename F>
    constexpr std::enable_if_t<!std::is_base_of_v<slot_base<Ret(Args...)>, F>,
                               ss::slot<Ret(Args...), sizeof(F)>>
    connect(F fn) noexcept
    {
        // make sure to ignore the this argument normally passed because there's
        // no containing object
        auto res = ss::slot<Ret(Args...), sizeof(F)>{
            [f = std::move(fn)](const slot<Ret(Args...)>* self,
                                auto&&... args) {
                (void) self;
                return f(std::forward<decltype(args)>(args)...);
            }};
        connect(res);
        return res;
    }
};
} // namespace ss