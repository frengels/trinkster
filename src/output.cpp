#include "output.hpp"

#include <algorithm>

#include "server.hpp"
#include "view.hpp"

struct render_data
{
    ::server*     server;
    wlr_output*   output;
    ::view*       view;
    wlr_renderer* renderer;
    timespec*     when;
};

static void render_surface(wlr_surface* surface, int sx, int sy, void* data)
{
    auto& rdata  = *static_cast<render_data*>(data);
    auto& server = *rdata.server;
    auto& view   = *rdata.view;
    auto* output = rdata.output;

    auto* texture = wlr_surface_get_texture(surface);

    if (!texture)
    {
        return;
    }

    double ox = 0.0, oy = 0.0;
    wlr_output_layout_output_coords(server.output_layout(), output, &ox, &oy);
    ox += view.x + sx;
    oy += view.y + sy;

    wlr_box box{static_cast<int>(ox * output->scale),
                static_cast<int>(oy * output->scale),
                static_cast<int>(surface->current.width * output->scale),
                static_cast<int>(surface->current.height * output->scale)};

    float matrix[9];
    auto  transform = wlr_output_transform_invert(surface->current.transform);
    wlr_matrix_project_box(
        matrix, &box, transform, 0, output->transform_matrix);

    wlr_render_texture_with_matrix(rdata.renderer, texture, matrix, 1);

    wlr_surface_send_frame_done(surface, rdata.when);
}

output::output(server* serv, wlr_output* output)
    : server_{serv}, wlr_output_{output}, frame_{[](auto* listener,
                                                    void* data) {
          (void) data;
          ::output* self     = wl_container_of(listener, self, frame_);
          auto*     renderer = self->server_->renderer();

          struct timespec now;
          clock_gettime(CLOCK_MONOTONIC, &now);

          if (!wlr_output_attach_render(self->wlr_output_, NULL))
          {
              return;
          }

          int width, height;
          wlr_output_effective_resolution(self->wlr_output_, &width, &height);

          wlr_renderer_begin(renderer, width, height);

          float color[4] = {0.3f, 0.3f, 0.3f, 1.0f};
          wlr_renderer_clear(renderer, color);

          std::for_each(self->server_->views().rbegin(),
                        self->server_->views().rend(),
                        [&](auto&& v) {
                            if (!v->mapped())
                            {
                                return;
                            }

                            render_data rdata{self->server_,
                                              self->wlr_output_,
                                              v.get(),
                                              renderer,
                                              &now};

                            wlr_xdg_surface_for_each_surface(
                                v->xdg_surface(), render_surface, &rdata);
                        });

          wlr_output_render_software_cursors(self->wlr_output_, nullptr);

          wlr_renderer_end(renderer);
          wlr_output_commit(self->wlr_output_);
      }}
{
    wl::connect(wlr_output_->events.frame, frame_);
}
