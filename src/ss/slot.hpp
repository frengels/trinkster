#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include <boost/callable_traits.hpp>

#include "ss/detail/intrusive_list.hpp"
#include "ss/detail/unaligned_storage.hpp"

namespace ss
{

template<typename Ret, typename... Args>
class slot_base;

template<typename Ret, typename... Args>
class slot_base<Ret(Args...)>
{
private:
    using function_type =
        std::add_pointer_t<Ret(const slot_base* self, Args...)>;

public:
    using result_type = Ret;
    using args_type   = std::tuple<const slot_base*, Args...>;

private:
    ss::detail::link link_{};
    function_type    fn_{nullptr};

public:
    static constexpr auto link_offset =
        offsetof(::ss::slot_base<Ret(Args...)>, link_);

public:
    constexpr slot_base() noexcept = default;

    constexpr slot_base(function_type fn) noexcept : fn_{fn}
    {}

    template<typename... UArgs>
    constexpr std::enable_if_t<std::is_invocable_r_v<result_type,
                                                     function_type,
                                                     const slot_base*,
                                                     UArgs...>,
                               result_type>
    operator()(UArgs&&... uargs) const
    {
        return fn_(this, std::forward<UArgs>(uargs)...);
    }

    constexpr void disconnect() noexcept
    {
        link_.remove();
    }
};

template<typename Sig, std::size_t Buff = 1>
class slot;

template<typename Ret, typename... Args, std::size_t Buff>
class slot<Ret(Args...), Buff> : public slot_base<Ret(Args...)>
{
    template<typename Sig_, std::size_t Buff_>
    friend class slot;

public:
    using result_type = Ret;
    using args_type   = std::tuple<Args...>;

private:
    /// this is the actual function being called
    ss::detail::unaligned_storage<Buff> callable_;

public:
    template<typename F>
    slot(F&& fn) noexcept
        : slot_base<Ret(Args...)>([](const slot_base<Ret(Args...)>* self,
                                     Args... args) -> Ret {
              const auto* this_ =
                  static_cast<const slot<Ret(Args...), Buff>*>(self);
              using functor_type = std::decay_t<F>;
              auto&& callable = this_->callable_.template get<functor_type>();

              if constexpr (std::is_same_v<void, Ret>)
              {
                  std::invoke(callable, this_, std::forward<Args>(args)...);
              }
              else
              {
                  return static_cast<Ret>(std::invoke(
                      callable, this_, std::forward<Args>(args)...));
              }
          })
    {
        using decayed_f = std::decay_t<F>;

        constexpr auto f_size      = sizeof(decayed_f);
        constexpr auto buffer_size = sizeof(callable_);
        static_assert(f_size <= buffer_size || std::is_empty_v<decayed_f>,
                      "callable F does not fit within the given buffer size.");

        static_assert(std::is_trivially_destructible_v<decayed_f>,
                      "callable F must be trivially destructible because of "
                      "type erasure.");
        static_assert(
            std::is_trivially_copyable_v<decayed_f>,
            "callable F must be trivially copyable to allow move operations.");
        static_assert(std::is_convertible_v<
                          std::invoke_result_t<decayed_f,
                                               slot<Ret(Args...), Buff>*,
                                               Args...>,
                          result_type>,
                      "Cannot convert invoke result to result_type.");

        callable_.template emplace<decayed_f>(std::forward<F>(fn));
    }
};
} // namespace ss