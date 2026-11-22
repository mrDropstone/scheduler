#pragma once

/* Generated with cbindgen:0.29.4 */

#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>
#include "private/slint_image_internal.h"
#include "private/slint_internal.h"

namespace slint { struct Rgb565Pixel; }
namespace slint::cbindgen_private {
    struct WindowProperties; using slint::Rgb565Pixel;
    using slint::cbindgen_private::types::TexturePixelFormat;
    struct DrawTextureArgs;
    struct DrawRectangleArgs;
    using types::PhysicalRegion;
}


namespace slint {
namespace cbindgen_private {

/// a Repr(C) variant of slint::platform::LayoutConstraints
struct LayoutConstraintsReprC {
    Size min;
    Size max;
    Size preferred;
    bool has_min;
    bool has_max;
};

using WindowAdapterUserData = void*;

struct RendererPtr {
    const void *_a;
    const void *_b;
};

using PlatformUserData = void*;

struct PlatformTaskOpaque {
    const void *_0;
    const void *_1;
};

using SoftwareRendererOpaque = const void*;

using CppTargetPixelBufferUserData = void*;

struct DrawRectangleArgs {
    float x;
    float y;
    float width;
    float height;
    float top_left_radius;
    float top_right_radius;
    float bottom_right_radius;
    float bottom_left_radius;
    float border_width;
    Brush background;
    Brush border;
    uint8_t alpha;
    /// 0, 90, 180, or 270
    int32_t rotation;
};

struct DrawTextureArgs {
    const uint8_t *image_data;
    TexturePixelFormat pixel_format;
    uintptr_t byte_stride;
    uint32_t width;
    uint32_t height;
    Color colorize;
    uint8_t alpha;
    intptr_t dst_x;
    intptr_t dst_y;
    uintptr_t dst_width;
    uintptr_t dst_height;
    /// 0, 90, 180, or 270
    int32_t rotation;
    bool has_tiling;
    int32_t tiling_offset_x;
    int32_t tiling_offset_y;
    float tiling_scale_x;
    float tiling_scale_y;
    uint32_t tiling_gap_x;
    uint32_t tiling_gap_y;
};

template<typename T>
struct CppTargetPixelBuffer {
    CppTargetPixelBufferUserData user_data;
    void (*line_slice)(CppTargetPixelBufferUserData, uintptr_t, T **slice_ptr, uintptr_t *slice_len);
    uintptr_t (*num_lines)(CppTargetPixelBufferUserData);
    bool (*fill_background)(CppTargetPixelBufferUserData, const Brush*, const PhysicalRegion*);
    bool (*draw_rectangle)(CppTargetPixelBufferUserData,
                           const DrawRectangleArgs*,
                           const PhysicalRegion*);
    bool (*draw_texture)(CppTargetPixelBufferUserData, const DrawTextureArgs*, const PhysicalRegion*);
};

using CppRawHandleOpaque = const void*;

using SkiaRendererOpaque = const void*;

/// Mirrors `EmscriptenWebGLContextAttributes` from `emscripten/html5_webgl.h`.
struct ContextAttributes {
    bool alpha;
    bool depth;
    bool stencil;
    bool antialias;
    bool premultiplied_alpha;
    bool preserve_drawing_buffer;
    int power_preference;
    bool fail_if_major_performance_caveat;
    int major_version;
    int minor_version;
    bool enable_extensions_by_default;
    bool explicit_swap_control;
    int proxy_context_to_main_thread;
    bool render_via_offscreen_back_buffer;
    bool desynchronized;
};

using DropCallback = void(*)(const void *obj);

using TranslateCallback = void(*)(const void *obj,
                                  Slice<uint8_t> string,
                                  Slice<uint8_t> context,
                                  SharedString *out);

using NTranslateCallback = void(*)(const void *obj,
                                   uint64_t n,
                                   Slice<uint8_t> singular,
                                   Slice<uint8_t> plural,
                                   Slice<uint8_t> context,
                                   SharedString *out);

extern "C" {

ColorScheme slint_context_color_scheme(const ItemTreeRc *root);

void slint_windowrc_init(WindowAdapterRcOpaque *out);

void slint_ensure_backend();

/// Enters the main event loop.
/// On Emscripten, it leaves `main()` by unwinding with a JavaScript exception; see
/// `emscripten_set_main_loop`.
void slint_run_event_loop(bool quit_on_last_window_closed);

/// Will execute the given functor in the main thread
void slint_post_event(void (*event)(void *user_data),
                      void *user_data,
                      void (*drop_user_data)(void*));

void slint_quit_event_loop();

void slint_register_font_from_path(const WindowAdapterRcOpaque *win,
                                   const SharedString *path,
                                   SharedString *error_str);

void slint_register_font_from_data(const WindowAdapterRcOpaque *win,
                                   Slice<uint8_t> data,
                                   SharedString *error_str);

void slint_register_bitmap_font(const WindowAdapterRcOpaque *win,
                                const BitmapFont<StaticStorage> *font_data);

bool slint_string_to_float(const SharedString *string, float *value);

bool slint_string_to_usize(const SharedString *string, uintptr_t *value);

void slint_debug(const SharedString *string);

void slint_set_xdg_app_id(const SharedString *_app_id);

/// Empty names and a null `graphics_api` leave the choice to the selector.
/// A `version_major` of 0 requests any version.
/// Panics if the requirements can't be met.
void slint_select_backend(const SharedString *backend_name,
                          const SharedString *renderer_name,
                          const GraphicsAPI *graphics_api,
                          uint8_t version_major,
                          uint8_t version_minor);

void slint_parse_markdown(const SharedString *format_string,
                          Slice<StyledText> args,
                          StyledText *out);

void slint_macos_bring_all_windows_to_front();

void slint_window_properties_get_title(const WindowProperties *wp, SharedString *out);

void slint_window_properties_get_background(const WindowProperties *wp, Brush *out);

bool slint_window_properties_get_fullscreen(const WindowProperties *wp);

bool slint_window_properties_get_minimized(const WindowProperties *wp);

bool slint_window_properties_get_maximized(const WindowProperties *wp);

LayoutConstraintsReprC slint_window_properties_get_layout_constraints(const WindowProperties *wp);

void slint_window_adapter_new(WindowAdapterUserData user_data,
                              void (*drop)(WindowAdapterUserData),
                              RendererPtr (*get_renderer_ref)(WindowAdapterUserData),
                              void (*set_visible)(WindowAdapterUserData, bool),
                              void (*request_redraw)(WindowAdapterUserData),
                              IntSize (*size)(WindowAdapterUserData),
                              void (*set_size)(WindowAdapterUserData, IntSize),
                              void (*update_window_properties)(WindowAdapterUserData,
                                                               const WindowProperties*),
                              bool (*position)(WindowAdapterUserData, Point2D<int32_t>*),
                              void (*set_position)(WindowAdapterUserData, Point2D<int32_t>),
                              WindowAdapterRcOpaque *target);

void slint_platform_register(PlatformUserData user_data,
                             void (*drop)(PlatformUserData),
                             void (*window_factory)(PlatformUserData, WindowAdapterRcOpaque*),
                             uint64_t (*duration_since_start)(PlatformUserData),
                             void (*set_clipboard_text)(PlatformUserData,
                                                        const SharedString*,
                                                        Clipboard),
                             bool (*clipboard_text)(PlatformUserData, SharedString*, Clipboard),
                             void (*run_event_loop)(PlatformUserData),
                             void (*quit_event_loop)(PlatformUserData),
                             void (*invoke_from_event_loop)(PlatformUserData, PlatformTaskOpaque));

bool slint_windowrc_has_active_animations(const WindowAdapterRcOpaque *handle);

void slint_platform_update_timers_and_animations();

/// Returns the duration in millisecond until the next timer or `u64::MAX` if there is no pending timers
uint64_t slint_platform_duration_until_next_timer_update();

void slint_platform_task_drop(PlatformTaskOpaque event);

void slint_platform_task_run(PlatformTaskOpaque event);

SoftwareRendererOpaque slint_software_renderer_new(uint32_t buffer_age);

void slint_software_renderer_drop(SoftwareRendererOpaque r);

PhysicalRegion slint_software_renderer_render_rgb8(SoftwareRendererOpaque r,
                                                   Rgb8Pixel *buffer,
                                                   uintptr_t buffer_len,
                                                   uintptr_t pixel_stride);

PhysicalRegion slint_software_renderer_render_accel_rgb8(SoftwareRendererOpaque r,
                                                         CppTargetPixelBuffer<Rgb8Pixel> *buffer);

PhysicalRegion slint_software_renderer_render_accel_rgb565(SoftwareRendererOpaque r,
                                                           CppTargetPixelBuffer<Rgb565Pixel> *buffer);

PhysicalRegion slint_software_renderer_render_rgb565(SoftwareRendererOpaque r,
                                                     uint16_t *buffer,
                                                     uintptr_t buffer_len,
                                                     uintptr_t pixel_stride);

PhysicalRegion slint_software_renderer_render_by_line_rgb565(SoftwareRendererOpaque r,
                                                             void (*process_line_fn)(void*,
                                                                                     uintptr_t,
                                                                                     uintptr_t,
                                                                                     uintptr_t,
                                                                                     void(*)(const void*,
                                                                                             Rgb565Pixel*,
                                                                                             uintptr_t),
                                                                                     const void*),
                                                             void *user_data);

PhysicalRegion slint_software_renderer_render_by_line_rgb8(SoftwareRendererOpaque r,
                                                           void (*process_line_fn)(void*,
                                                                                   uintptr_t,
                                                                                   uintptr_t,
                                                                                   uintptr_t,
                                                                                   void(*)(const void*,
                                                                                           Rgb8Pixel*,
                                                                                           uintptr_t),
                                                                                   const void*),
                                                           void *user_data);

void slint_software_renderer_set_rendering_rotation(SoftwareRendererOpaque r, int32_t rotation);

void slint_software_renderer_set_dirty_region_alignment(SoftwareRendererOpaque r,
                                                        uint16_t horizontal,
                                                        uint16_t vertical);

RendererPtr slint_software_renderer_handle(SoftwareRendererOpaque r);

void slint_software_renderer_region_to_rects(const PhysicalRegion *region,
                                             SharedVector<IntRect> *out);

CppRawHandleOpaque slint_new_raw_window_handle_win32(void *hwnd, void *_hinstance);

CppRawHandleOpaque slint_new_raw_window_handle_x11_xcb(uint32_t window,
                                                       uint32_t visual_id,
                                                       void *connection,
                                                       int screen);

CppRawHandleOpaque slint_new_raw_window_handle_x11_xlib(unsigned long window,
                                                        unsigned long visual_id,
                                                        void *display,
                                                        int screen);

CppRawHandleOpaque slint_new_raw_window_handle_wayland(void *surface, void *display);

CppRawHandleOpaque slint_new_raw_window_handle_appkit(void *ns_view, void *_ns_window);

void slint_raw_window_handle_drop(CppRawHandleOpaque handle);

SkiaRendererOpaque slint_skia_renderer_new(CppRawHandleOpaque handle_opaque, IntSize size);

void slint_skia_renderer_drop(SkiaRendererOpaque r);

void slint_skia_renderer_render(SkiaRendererOpaque r);

RendererPtr slint_skia_renderer_handle(SkiaRendererOpaque r);

void slint_em_on_pointer(int kind, float x, float y, int button, bool is_touch);

void slint_em_on_wheel(float x, float y, float delta_x, float delta_y);

/// Returns whether the key maps to a Slint key, and so was dispatched.
bool slint_em_on_key(bool pressed, const char *key, bool shift, bool repeat);

void slint_em_on_focus(bool focused);

void slint_em_on_resize(float width, float height, float scale_factor);

/// With `simulate_infinite_loop`, this unwinds with a JavaScript exception to leave `main()`.
extern void emscripten_set_main_loop(void (*func)(), int fps, bool simulate_infinite_loop);

extern void emscripten_cancel_main_loop();

extern int slint_em_attach_canvas();

extern void slint_em_canvas_css_size(float *width, float *height, float *ratio);

extern void slint_em_set_canvas_css_size(float width, float height);

extern void slint_em_set_canvas_buffer_size(uint32_t width, uint32_t height);

extern void slint_em_set_cursor(const char *name);

extern void slint_em_set_title(const char *title);

extern void slint_em_put_image_data(const uint8_t *pixels, uint32_t width, uint32_t height);

extern void emscripten_webgl_init_context_attributes(ContextAttributes *attributes);

extern uintptr_t emscripten_webgl_create_context(const char *target,
                                                 const ContextAttributes *attributes);

extern int emscripten_webgl_make_context_current(uintptr_t context);

extern const void *emscripten_webgl_get_proc_address(const char *name);

#if defined(__ANDROID__)
extern void slint_main();
#endif

bool slint_translate_set_translator(const void *obj,
                                    DropCallback drop,
                                    TranslateCallback translate,
                                    NTranslateCallback ntranslate);

}  // extern "C"

}  // namespace cbindgen_private
}  // namespace slint
