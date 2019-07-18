#include "listener.hpp"

namespace wl
{
void connect(wl_signal& s, wl_listener& l) noexcept
{
    wl_signal_add(&s, &l);
}

void connect(wl_signal& s, wl::listener& l) noexcept
{
    wl_list_insert(s.listener_list.prev, &l.link);
}

listener::listener(listener&& other) noexcept : wl_listener{other}
{
    // we need to set these again for our surrounding as we just moved
    // ourself
    this->link.prev->next = &this->link;
    this->link.next->prev = &this->link;
}

listener& listener::operator=(listener&& other) noexcept
{
    remove();

    this->notify = other.notify;

    // fix up the links
    this->link            = other.link;
    this->link.prev->next = &this->link;
    this->link.next->prev = &this->link;

    // unlink the other listener to prevent removal
    other.link.prev = nullptr;
    other.link.next = nullptr;

    return *this;
}

void listener::remove() noexcept
{
    wl_list_remove(&this->link);
}

void listener::swap(listener& other) noexcept
{
    auto tmp_link = this->link;

    this->link            = other.link;
    this->link.prev->next = &this->link;
    this->link.next->prev = &this->link;

    other.link            = tmp_link;
    other.link.prev->next = &other.link;
    other.link.next->prev = &other.link;
}
} // namespace wl