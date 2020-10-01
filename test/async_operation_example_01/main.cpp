#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/write.hpp>
#include <cstring>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

// The return type of the initiating function is deduced from the combination of
// CompletionToken type and the completion handle's signature. When the
// completion token is a simple callback, the return type is void. However, when
// the completion toke is boost::asio::yield_context (use for stackful
// coroutines) the return type would be std::size_t, and when the completion
// token is boost::asio::use_future it would be std::future<std::size_t>.
template <typename CompletionToken>
auto async_write_message(boost::asio::ip::tcp::socket &socket,
                         const char *message, CompletionToken &&token) {
  // When delegating to the underlying operation we must take care to perfectly
  // forward completion token. This ensures that out operation works correctly
  // with move-only function objects as callbacks, as well as other completion
  // token types.
  return boost::asio::async_write(
      socket, boost::asio::buffer(message, std::strlen(message)),
      std::forward<CompletionToken>(token));
}

void test_callback() {
  boost::asio::io_context io_context;
  boost::asio::ip::tcp::acceptor acceptor(io_context,
                                          {boost::asio::ip::tcp::v4(), 1234});
  boost::asio::ip::tcp::socket socket = acceptor.accept();

  // Test our asynchronous operation using a lambda as a callback.
  async_write_message(socket, "Testing callback\n",
                      [](const boost::system::error_code &err, std::size_t n) {
                        if (!err) {
                          std::cout << n << " bytes tranferred" << std::endl;
                        } else {
                          std::cout << "Error: " << err.message() << std::endl;
                        }
                      });
  io_context.run();
}

void test_future() {
  boost::asio::io_context io_context;
  boost::asio::ip::tcp::acceptor acceptor(io_context,
                                          {boost::asio::ip::tcp::v4(), 1234});
  boost::asio::ip::tcp::socket socket = acceptor.accept();

  // Test our asynchronous operation using the use_future completion token. This
  // token causes the operation's initiating function to return a future, which
  // may be used to synchronously wait for the result of operation.
  std::future<std::size_t> f =
      async_write_message(socket, "Testing future\n", boost::asio::use_future);
  io_context.run();
  try {
    std::size_t n = f.get();
    std::cout << n << " bytes transferred" << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }
}

int main() {
  test_callback();
  test_future();
  return 0;
}
