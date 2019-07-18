#include "keyboard.hpp"

#include "server.hpp"

keyboard::keyboard(server* serv, wlr_input_device* device)
    : server_{serv}, device_{device}, modifiers_{[](auto* listener, void*) {
          keyboard* self = wl_container_of(listener, self, modifiers_);

          wlr_seat_set_keyboard(self->server_->seat(), self->device_);
          wlr_seat_keyboard_notify_modifiers(
              self->server_->seat(), &self->device_->keyboard->modifiers);
      }},
      key_{[](auto* listener, void* data) {
          keyboard* self   = wl_container_of(listener, self, key_);
          auto*     server = self->server_;
          auto*     event  = static_cast<wlr_event_keyboard_key*>(data);
          auto*     seat   = server->seat();

          // translate libinput to xkbcommon keycode
          uint32_t keycode = event->keycode + 8;
          // get a list of keysyms based on the keymap for this keyboard
          const xkb_keysym_t* syms;

          int nsyms = xkb_state_key_get_syms(
              self->device_->keyboard->xkb_state, keycode, &syms);
          (void) nsyms;

          bool     handled = false;
          uint32_t modifiers =
              wlr_keyboard_get_modifiers(self->device_->keyboard);

          if ((modifiers & WLR_MODIFIER_LOGO) &&
              event->state == WLR_KEY_PRESSED)
          {
              // handle super presses as compositor events
              // TODO handle these keybinds
          }

          if (!handled)
          {
              wlr_seat_set_keyboard(seat, self->device_);
              wlr_seat_keyboard_notify_key(
                  seat, event->time_msec, event->keycode, event->state);
          }
      }}
{
    xkb_rule_names rules{};
    xkb_context*   context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    xkb_keymap*    keymap =
        xkb_map_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(device->keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    wlr_keyboard_set_repeat_info(device->keyboard, 25, 600);

    wl::connect(device_->keyboard->events.modifiers, modifiers_);
    wl::connect(device_->keyboard->events.key, key_);

    wlr_seat_set_keyboard(serv->seat(), device_);
}