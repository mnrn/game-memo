#include <jemalloc/jemalloc.h>
#include <uv.h>

#include <cstdlib>
#include <iostream>

static constexpr int DEFAULT_PORT = 1234;
static constexpr int DEFAULT_BACKLOG = 128;
static uv_loop_t *loop = nullptr;

struct write_req {
  uv_write_t req;
  uv_buf_t buf;
};

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread > 0) {
    write_req *req = static_cast<write_req *>(jemalloc(sizeof(write_req)));
    req->buf = uv_buf_init(buf->base, nread);
    uv_write(reinterpret_cast<uv_write_t *>(req), client, &req->buf, 1,
             [](uv_write_t *req, int stat) {
               if (stat != 0) {
                 std::cerr << "Write error " << uv_strerror(stat) << std::endl;
               }
               write_req *wr = reinterpret_cast<write_req *>(req);
               jefree(wr->buf.base);
               jefree(wr);
             });
    return;
  }
  if (nread < 0) {
    if (nread != UV_EOF) {
      std::cerr << "Read error " << uv_err_name(nread) << std::endl;
    }
    uv_close(reinterpret_cast<uv_handle_t *>(client),
             [](uv_handle_t *h) { jefree(h); });
  }
  jefree(buf->base);
}

void on_new_connection(uv_stream_t *server, int stat) {
  if (stat < 0) {
    std::cerr << "New connection error " << uv_strerror(stat) << std::endl;
    return;
  }
  uv_tcp_t *client = static_cast<uv_tcp_t *>(jemalloc(sizeof(uv_tcp_t)));
  uv_tcp_init(loop, client);
  if (uv_accept(server, reinterpret_cast<uv_stream_t *>(client)) == 0) {
    uv_read_start(
        reinterpret_cast<uv_stream_t *>(client),
        [](uv_handle_t *, size_t suggested_size, uv_buf_t *buf) {
          buf->base = static_cast<char *>(jemalloc(suggested_size));
          buf->len = suggested_size;
        },
        echo_read);
  } else {
    uv_close(reinterpret_cast<uv_handle_t *>(client),
             [](uv_handle_t *h) { jefree(h); });
  }
}

int main() {
  loop = uv_default_loop();

  uv_tcp_t server;
  uv_tcp_init(loop, &server);

  sockaddr_in6 addr;
  uv_ip6_addr("::", DEFAULT_PORT, &addr);

  uv_tcp_bind(&server, reinterpret_cast<const sockaddr *>(&addr), 0);
  const int r = uv_listen(reinterpret_cast<uv_stream_t *>(&server),
                          DEFAULT_BACKLOG, on_new_connection);
  if (r != 0) {
    std::cerr << "Listen error " << uv_strerror(r) << std::endl;
    return EXIT_FAILURE;
  }
  return uv_run(loop, UV_RUN_DEFAULT);
}
