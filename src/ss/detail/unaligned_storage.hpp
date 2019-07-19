#pragma once

#include <cstddef>
#include <iterator>

namespace ss
{
namespace detail
{
template<std::size_t Len>
class unaligned_storage
{
    static_assert(Len >= 1, "The minimum Len value supported is 1");

    template<std::size_t Len_>
    friend class unaligned_storage;

private:
    std::byte storage_[Len];

public:
    constexpr unaligned_storage() noexcept = default;

    template<typename T>
    std::enable_if_t<sizeof(T) <= Len, T&> get() noexcept
    {
        return *reinterpret_cast<T*>(&storage_);
    }

    template<typename T>
    std::enable_if_t<sizeof(T) <= Len, const T&> get() const noexcept
    {
        return *reinterpret_cast<const T*>(storage_);
    }

    template<typename T, typename... Args>
    std::enable_if_t<sizeof(T) <= Len || std::is_empty_v<T>, T&> emplace(
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        new (storage_) T(std::forward<Args>(args)...);
        return get<T>();
    }
};
} // namespace detail
} // namespace ss