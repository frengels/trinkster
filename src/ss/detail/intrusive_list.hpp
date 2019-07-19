#pragma once

#include <cstddef>
#include <memory>

namespace ss
{
namespace detail
{
// intrusive traits must contain the offset of the link within the T
template<typename T>
struct intrusive_link_offset
    : std::integral_constant<std::size_t, T::link_offset>
{};

template<typename T>
constexpr auto intrusive_link_offset_v = intrusive_link_offset<T>::value;

// these are implemented as circular links, a link null link always refers to
// itself.
class link
{
public:
    link* prev{this};
    link* next{this};

public:
    constexpr link() noexcept = default;
    constexpr link(link* prev, link* next) noexcept : prev{prev}, next{next}
    {}

    // copies are disallowed
    constexpr link(const link&) = delete;
    constexpr link& operator=(const link&) = delete;

    constexpr link(link&& other) noexcept
        : prev{other.prev == &other ? this : other.prev}, next{other.next ==
                                                                       &other ?
                                                                   this :
                                                                   other.next}
    {
        other.next = &other; // clear other
        other.prev = &other;

        prev->next = this; // reassign pointers
        next->prev = this;
    }

    constexpr link& operator=(link&& other) noexcept
    {
        remove();

        if (other.prev != &other)
        {
            prev       = other.prev;
            prev->next = this;
        }

        if (other.next != &other)
        {
            next       = other.next;
            next->prev = this;
        }

        other.prev = &other;
        other.next = &other;

        return *this;
    }

    ~link()
    {
        remove();
    }

    explicit constexpr operator bool() const noexcept
    {
        return prev != this && next != this;
    }

    constexpr void remove() noexcept
    {
        prev->next = next;
        next->prev = prev;

        prev = this;
        next = this;
    }

    constexpr void swap(link& other) noexcept
    {
        // prev pointer
        auto* tmp = prev;
        if (other.prev == &other) // self referencing
        {
            prev = this;
        }
        else
        {
            prev       = other.prev;
            prev->next = this;
        }

        if (tmp == this)
        {
            other.prev = &other;
        }
        else
        {
            other.prev       = tmp;
            other.prev->next = &other;
        }

        // next pointer
        tmp = next;
        if (other.next == &other) // self referencing == null
        {
            next = this;
        }
        else
        {
            next       = other.next;
            next->prev = this;
        }

        if (tmp == this)
        {
            other.next = &other;
        }
        else
        {
            other.next       = tmp;
            other.next->prev = &other;
        }
    }
}; // namespace detail

template<typename T>
class intrusive_list
{
public:
    static constexpr auto offset = intrusive_link_offset_v<T>;

    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;

private:
    template<typename Link, typename U>
    class iterator_base
    {
        // give all iterator_base access amongst each other
        template<typename Link_, typename U_>
        friend class iterator_base;

    public:
        using value_type        = std::remove_const_t<U>;
        using reference         = U&;
        using pointer           = U*;
        using difference_type   = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

    private:
        Link current_{nullptr};

    public:
        constexpr iterator_base() noexcept = default;

        constexpr iterator_base(Link curr) noexcept : current_{curr}
        {}

        template<typename Link_, typename U_>
        constexpr iterator_base(const iterator_base<Link_, U_>& other) noexcept
            : current_{other.current_}
        {
            static_assert(std::is_convertible_v<
                              typename iterator_base<Link_, U_>::reference,
                              reference>,
                          "Incompatible reference types");
            static_assert(std::is_convertible_v<
                              typename iterator_base<Link_, U_>::pointer,
                              pointer>,
                          "Incompatible pointer types");
        }

        template<typename Link_, typename U_>
        constexpr iterator_base&
        operator=(const iterator_base<Link_, U_>& other) noexcept
        {
            static_assert(std::is_convertible_v<
                              typename iterator_base<Link_, U_>::reference,
                              reference>,
                          "Incompatible reference types");
            static_assert(std::is_convertible_v<
                              typename iterator_base<Link_, U_>::pointer,
                              pointer>,
                          "Incompatible pointer types");

            current_ = other.current_;
            return *this;
        }

        template<typename Link_, typename U_>
        constexpr bool operator==(const iterator_base<Link_, U_>& other) const
            noexcept
        {
            return current_ == other.current_;
        }

        template<typename Link_, typename U_>
        constexpr bool operator!=(const iterator_base<Link_, U_>& other) const
            noexcept
        {
            return !(*this == other);
        }

        constexpr iterator_base& operator++() noexcept
        {
            current_ = current_->next;
            return *this;
        }

        constexpr iterator_base operator++(int) noexcept
        {
            auto res = *this;
            ++(*this);
            return res;
        }

        constexpr iterator_base& operator--() noexcept
        {
            current_ = current_->prev;
            return *this;
        }

        constexpr iterator_base operator--(int) noexcept
        {
            auto res = *this;
            --(*this);
            return res;
        }

        reference operator*() const noexcept
        {
            if constexpr (std::is_const_v<std::remove_pointer_t<Link>>)
            {
                const auto* ptr = reinterpret_cast<const char*>(current_);
                ptr -= offset;
                const auto* obj = reinterpret_cast<pointer>(ptr);
                return *obj;
            }
            else
            {
                // calculate back from the link to the actual object
                auto* ptr = reinterpret_cast<char*>(current_);
                ptr -= offset; // this is the location of our actual object
                auto* obj = reinterpret_cast<pointer>(ptr);
                return *obj;
            }
        }

        pointer operator->() const noexcept
        {
            auto* ptr = reinterpret_cast<char*>(current_);
            ptr -= offset;
            auto* obj = reinterpret_cast<pointer>(ptr);
            return obj;
        }
    };

    class iterator : public iterator_base<link*, T>
    {
        using iterator_base<link*, T>::iterator_base;
    };

    class const_iterator : public iterator_base<const link*, const T>
    {
        using iterator_base<const link*, const T>::iterator_base;
    };

    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    // holds reference to the first and last link
    detail::link link_;

public:
    constexpr intrusive_list() noexcept = default;

    constexpr iterator begin() noexcept
    {
        return {link_.next};
    }

    constexpr iterator end() noexcept
    {
        return {&link_};
    }

    constexpr const_iterator begin() const noexcept
    {
        return const_iterator{link_.next};
    }

    constexpr const_iterator end() const noexcept
    {
        return const_iterator{&link_};
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return {link_.next};
    }

    constexpr const_iterator cend() const noexcept
    {
        return {&link_};
    }

    void push_back(T& ref) noexcept
    {
        char* ptr = reinterpret_cast<char*>(std::addressof(ref));
        ptr += offset; // this is where the link is located
        link* l = reinterpret_cast<link*>(ptr);

        link_.prev->next = l;
        l->prev          = link_.prev;

        link_.prev = l;
        l->next    = &link_;
    }

    constexpr bool empty() const noexcept
    {
        return link_.next == &link_;
    }

    constexpr void clear() noexcept
    {
        auto* current = &link_;

        // we need to remove all links from the list
        do
        {
            auto* next    = current->next;
            current->prev = current;
            current->next = current;
            current       = next;
        } while (current != &link_);
    }
};
} // namespace detail
} // namespace ss