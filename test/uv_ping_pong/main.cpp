#include <jemalloc/jemalloc.h>
#include <uv.h>

#include <boost/assert.hpp>
#include <iostream>
#include <string>
#include <type_traits>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

static constexpr int BUFSIZE = 10240;
static constexpr int NUM_PINGS = 100;
static char PING[] = "PING\n";
static constexpr int TEST_PORT = 1234;
static int pinger_on_connect_count = 0;
static int completed_pingers = 0;

struct pinger_t {
  int vectored_writes;
  int pongs;
  int state;
  union {
    uv_tcp_t tcp;
    uv_pipe_t pipe;
  } stream;
  uv_connect_t con_req;
  char read_buf[BUFSIZE];
};
static_assert(std::is_pod_v<pinger_t>);

static inline void close_loop(uv_loop_t *loop) {
  uv_walk(
      loop,
      [](uv_handle_t *handle, void *) {
        if (!uv_is_closing(handle)) {
          uv_close(handle, nullptr);
        }
      },
      nullptr);
  uv_run(loop, UV_RUN_DEFAULT);
}

static inline bool can_ipv6() {
  uv_interface_address_t *addr = nullptr;
  int count = 0;
  if (uv_interface_addresses(&addr, &count)) {
    return false;
  }

  bool supported = false;
  for (int i = 0; supported == false && i < count; i++) {
    supported = (addr[i].address.address6.sin6_family == AF_INET6);
  }
  uv_free_interface_addresses(addr, count);
  return supported;
}

static void pinger_write_ping(pinger_t *pinger) {
  uv_buf_t bufs[sizeof(PING) - 1]{};
  int nbufs = 0;
  if (pinger->vectored_writes == 0) {
    // Wriet a single buffer.
    nbufs = 1;
    bufs[0] = uv_buf_init(PING, sizeof(PING) - 1);
  } else {
    // Write multiple buffers, each with one byte in them.
    nbufs = sizeof(PING) - 1;
    for (int i = 0; i < nbufs; i++) {
      bufs[i] = uv_buf_init(&PING[i], 1);
    }
  }

  uv_write_t *req = static_cast<uv_write_t *>(jemalloc(sizeof(uv_write_t)));
  if (uv_write(req, reinterpret_cast<uv_stream_t *>(&pinger->stream.tcp), bufs,
               nbufs, [](uv_write_t *req, int stat) {
                 BOOST_ASSERT(stat == 0);
                 jefree(req);
               })) {
    BOOST_ASSERT_MSG(false, "uv_write faileed");
  }

  std::cout << "PING" << std::endl;
}

static void pinger_read_cb(uv_stream_t *stream, ssize_t nread,
                           const uv_buf_t *buf) {

  pinger_t *pinger = static_cast<pinger_t *>(stream->data);
  auto pinger_on_close = [](uv_handle_t *handle) {
    pinger_t *pinger = static_cast<pinger_t *>(handle->data);

    BOOST_ASSERT(NUM_PINGS == pinger->pongs);

    jefree(pinger);
    completed_pingers++;
  };

  if (nread < 0) {
    BOOST_ASSERT(nread == UV_EOF);
    std::cout << "got EOF" << std::endl;

    jefree(buf->base);
    uv_close(reinterpret_cast<uv_handle_t *>(&pinger->stream.tcp),
             pinger_on_close);
    return;
  }

  // Now we count the pings.
  for (int i = 0; i < nread; i++) {
    BOOST_ASSERT(buf->base[i] == PING[pinger->state]);
    pinger->state = (pinger->state + 1) % (sizeof(PING) - 1);

    if (pinger->state != 0) {
      continue;
    }

    fmt::print("PONG {}\n", pinger->pongs);
    pinger->pongs++;

    if (pinger->pongs < NUM_PINGS) {
      pinger_write_ping(pinger);
    } else {
      uv_close(reinterpret_cast<uv_handle_t *>(&pinger->stream.tcp),
               pinger_on_close);
      break;
    }
  }
  jefree(buf->base);
}

static void pinger_on_connect(uv_connect_t *req, int stat) {
  pinger_t *pinger = static_cast<pinger_t *>(req->handle->data);

  pinger_on_connect_count++;

  BOOST_ASSERT(stat == 0);

  BOOST_ASSERT(uv_is_readable(req->handle) == 1);
  BOOST_ASSERT(uv_is_writable(req->handle) == 1);
  BOOST_ASSERT(uv_is_closing(reinterpret_cast<uv_handle_t *>(req->handle)) ==
               0);

  pinger_write_ping(pinger);

  uv_read_start(
      static_cast<uv_stream_t *>(req->handle),
      [](uv_handle_t *, size_t size, uv_buf_t *buf) {
        buf->base = static_cast<char *>(jemalloc(size));
        buf->len = size;
      },
      pinger_read_cb);
}

// same ping-pong test, but using IPv6 connection.
static void tcp_pinger_v6_new(int vectored_writes) {
  sockaddr_in6 server_addr;
  BOOST_ASSERT(0 == uv_ip6_addr("::1", TEST_PORT, &server_addr));
  pinger_t *pinger = static_cast<pinger_t *>(jemalloc(sizeof(pinger_t)));
  BOOST_ASSERT(pinger != nullptr);

  pinger->vectored_writes = vectored_writes;
  pinger->state = 0;
  pinger->pongs = 0;

  pinger_on_connect_count = 0;
  completed_pingers = 0;

  // Try to connect to the server and do NUM_PINGS ping-pong;
  int r = uv_tcp_init(uv_default_loop(), &pinger->stream.tcp);
  pinger->stream.tcp.data = pinger;
  BOOST_ASSERT(r == 0);

  // We are never doing multiple reads/connects at a time anyway, so these
  // handles can be pre-initialized.
  r = uv_tcp_connect(&pinger->con_req, &pinger->stream.tcp,
                     reinterpret_cast<const sockaddr *>(&server_addr),
                     pinger_on_connect);
  BOOST_ASSERT(r == 0);

  BOOST_ASSERT_MSG(pinger_on_connect_count == 0,
                   "Synchronous connect callbacks are not allowed.");
}

static void tcp_pinger_new(int vectored_writes) {
  sockaddr_in server_addr;
  BOOST_ASSERT(0 == uv_ip4_addr("127.0.0.1", TEST_PORT, &server_addr));
  pinger_t *pinger = static_cast<pinger_t *>(jemalloc(sizeof(pinger_t)));
  BOOST_ASSERT(pinger != nullptr);

  pinger->vectored_writes = vectored_writes;
  pinger->state = 0;
  pinger->pongs = 0;

  pinger_on_connect_count = 0;
  completed_pingers = 0;

  // Try to connect to the server and do NUM_PINGS ping-pong;
  int r = uv_tcp_init(uv_default_loop(), &pinger->stream.tcp);
  pinger->stream.tcp.data = pinger;
  BOOST_ASSERT(r == 0);

  // We are never doing multiple reads/connects at a time anyway, so these
  // handles can be pre-initialized.
  r = uv_tcp_connect(&pinger->con_req, &pinger->stream.tcp,
                     reinterpret_cast<const sockaddr *>(&server_addr),
                     pinger_on_connect);
  BOOST_ASSERT(r == 0);

  BOOST_ASSERT_MSG(pinger_on_connect_count == 0,
                   "Synchronous connect callbacks are not allowed.");
}

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("tcp-ping-pong") {
  SECTION("tcp-ping-pong") {
    tcp_pinger_new(0);
    REQUIRE(uv_run(uv_default_loop(), UV_RUN_DEFAULT) == 0);
    close_loop(uv_default_loop());
    REQUIRE(uv_loop_close(uv_default_loop()) == 0);
  }
  SECTION("tcp-ping-pong-vec") {
    tcp_pinger_new(1);
    REQUIRE(uv_run(uv_default_loop(), UV_RUN_DEFAULT) == 0);
    close_loop(uv_default_loop());
    REQUIRE(uv_loop_close(uv_default_loop()) == 0);
  }
  SECTION("ipv6") {
    REQUIRE(can_ipv6());
    /*
    SECTION("tcp6-ping-pong") {
      tcp_pinger_v6_new(0);
      REQUIRE(uv_run(uv_default_loop(), UV_RUN_DEFAULT) == 0);
      close_loop(uv_default_loop());
      REQUIRE(uv_loop_close(uv_default_loop()) == 0);
    }
    SECTION("tcp6-ping-pong-vec") {
      tcp_pinger_v6_new(1);
      REQUIRE(uv_run(uv_default_loop(), UV_RUN_DEFAULT) == 0);
      close_loop(uv_default_loop());
      REQUIRE(uv_loop_close(uv_default_loop()) == 0);
    }
    */
  }
}
