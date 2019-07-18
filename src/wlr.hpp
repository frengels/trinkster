#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    //#define namespace _namespace
    //#define class _class

#include <wlr/backend.h>
#include <wlr/render/egl.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_box.h>
#define static
#include <wlr/render/wlr_renderer.h>
#undef static
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#define static
#include <wlr/types/wlr_matrix.h>
#undef static
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

    //#undef class
    //#undef namespace

#ifdef __cplusplus
}
#endif
