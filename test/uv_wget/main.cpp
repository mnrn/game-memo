#include <boost/assert.hpp>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <jemalloc/jemalloc.h>
#include <uv.h>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

static uv_loop_t *loop = nullptr;
static CURLM *curl_handle = nullptr;
static uv_timer_t timeout;

typedef struct curl_context_s {
  uv_poll_t poll_handle;
  curl_socket_t sockfd;
} curl_context_t;

static curl_context_t *create_curl_context(curl_socket_t sockfd) {
  curl_context_t *context = nullptr;
  context = static_cast<curl_context_t *>(jemalloc(sizeof(curl_context_t)));
  context->sockfd = sockfd;

  const int r = uv_poll_init_socket(loop, &context->poll_handle, sockfd);
  BOOST_ASSERT(r == 0);
  context->poll_handle.data = context;
  return context;
}

static void check_multi_info() {
  int pending = 0;
  char *done_url = nullptr;
  while (CURLMsg *msg = curl_multi_info_read(curl_handle, &pending)) {
    switch (msg->msg) {
    case CURLMSG_DONE:
      curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &done_url);
      std::cout << done_url << " DONE" << std::endl;
      curl_multi_remove_handle(curl_handle, msg->easy_handle);
      curl_easy_cleanup(msg->easy_handle);
      break;
    default:
      BOOST_ASSERT_MSG(false, "CURLMSG default");
    }
  }
}

static void curl_perform(uv_poll_t *req, int status, int events) {
  uv_timer_stop(&timeout);
  int flags = 0;
  if (status < 0) {
    flags = CURL_CSELECT_ERR;
  }
  if (!status && events & UV_READABLE) {
    flags |= CURL_CSELECT_IN;
  }
  if (!status && events & UV_WRITABLE) {
    flags |= CURL_CSELECT_OUT;
  }

  curl_context_t *context = reinterpret_cast<curl_context_t *>(req);
  int running_handles = 0;
  curl_multi_socket_action(curl_handle, context->sockfd, flags,
                           &running_handles);
  check_multi_info();
}

static void start_timeout(CURLM *, long timeout_ms, void *) {
  if (timeout_ms <= 0) {
    timeout_ms =
        1; // 0 means directly call socket_action, but we'll do it in a bit.
  }
  uv_timer_start(
      &timeout,
      [](uv_timer_t *) {
        int running_handles;
        curl_multi_socket_action(curl_handle, CURL_SOCKET_TIMEOUT, 0,
                                 &running_handles);
        check_multi_info();
      },
      timeout_ms, 0);
}

static void add_download(const char *url, int index) {
  const std::string filename = fmt::format("{}.download", index);
  FILE *fp = fopen(filename.c_str(), "w");
  if (fp == nullptr) {
    std::cerr << "Error opening" << filename << std::endl;
    return;
  }
  CURL *handle = curl_easy_init();
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, fp);
  curl_easy_setopt(handle, CURLOPT_URL, url);
  curl_multi_add_handle(curl_handle, handle);
  std::cerr << "Added download " << url << "-> " << filename << std::endl;
}

static int handle_socket(CURL *, curl_socket_t s, int action, void *,
                         void *socketp) {
  curl_context_t *curl_context = nullptr;
  if (action == CURL_POLL_IN || action == CURL_POLL_OUT) {
    if (socketp != nullptr) {
      curl_context = static_cast<curl_context_t *>(socketp);
    } else {
      curl_context = create_curl_context(s);
      curl_multi_assign(curl_handle, s, curl_context);
    }
  }

  switch (action) {
  case CURL_POLL_IN:
    uv_poll_start(&curl_context->poll_handle, UV_READABLE, curl_perform);
    break;
  case CURL_POLL_OUT:
    uv_poll_start(&curl_context->poll_handle, UV_WRITABLE, curl_perform);
    break;
  case CURL_POLL_REMOVE:
    if (socketp != nullptr) {
      uv_poll_stop(&static_cast<curl_context_t *>(socketp)->poll_handle);
      uv_close(reinterpret_cast<uv_handle_t *>(&curl_context->poll_handle),
               [](uv_handle_t *h) {
                 curl_context_t *context =
                     static_cast<curl_context_t *>(h->data);
                 jefree(context);
               });
    }
    break;
  default:
    BOOST_ASSERT_MSG(false, "CURL_POLL default");
  }
  return 0;
}

int main(int argc, char **argv) {
  if (argc <= 1) {
    std::cerr << "Usage: uv_wget <filename> ..." << std::endl;
    return EXIT_FAILURE;
  }
  loop = uv_default_loop();
  if (curl_global_init(CURL_GLOBAL_ALL)) {
    std::cerr << "Could not init cURL !" << std::endl;
    return EXIT_FAILURE;
  }

  uv_timer_init(loop, &timeout);
  curl_handle = curl_multi_init();
  curl_multi_setopt(curl_handle, CURLMOPT_SOCKETFUNCTION, handle_socket);
  curl_multi_setopt(curl_handle, CURLMOPT_TIMERFUNCTION, start_timeout);

  while (argc-- > 1) {
    add_download(argv[argc], argc);
  }

  uv_run(loop, UV_RUN_DEFAULT);
  curl_multi_cleanup(curl_handle);
  return 0;
}
