#include <boost/asio/compose.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/write.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

// The return type of the initiating function is deduced from the combination of
// CompletionToken type and the completion handle's signature. When the
// completion token is a simple callback, the return type is void. However, when
// the completion token is boost::asio::yield_context (use for stackful
// coroutines) the return type would be also be void, as there is no non-error
// argument to the completion handler. When the completion token is
// boost::asio::use_future it would be std::future<void>.
template <typename T, typename CompletionToken>
auto async_write_message(boost::asio::ip::tcp::socket &socket, const T &message,
                         std::size_t repeat_count, CompletionToken &&token) {
  // Encoded the message and copy it into an allocated buffer. The buffer
  // will be maintained for the lifetime of the asynchornous operation.
  std::ostringstream os;
  os << message;
  std::unique_ptr<std::string> encoded_message =
      std::make_unique<std::string>(os.str());

  // Create a steady_timer to be used for the delay between messages.
  std::unique_ptr<boost::asio::steady_timer> delay_timer =
      std::make_unique<boost::asio::steady_timer>(socket.get_executor());

  // To manage the cycle between the multiple underlying asynchronous
  // operations, our implemention is a state machine.
  enum struct state {
    starting,
    waiting,
    writing,
  };

  // The boost::asio::async_compose function takes:
  //
  // - our asynchronous operatioon implementation,
  // - the completion token,
  // - the completion handler signature, and
  // - any I/O objects (or executors) used by the operation
  //
  // If then wraps our implementation, which is implemented here as a
  // state machine in a lambda, in an intermediate completion handler that
  // meets the requirements of a confornubg asynchronous operation. This
  // includes tracking outstanding work against the I/O executors
  // associated with the operation (in this example, this is the socket's
  // executor).
  //
  // The first argument to our lanbda is a reference to the enclosing
  // intermediate completion handler. This intermediate completion handler
  // is provided for us by the boost::asio::async_compose function, and
  // takes care of all the details required to implement a conforming
  // asynchronous operation. When calling an underlying asynchronous
  // operation, we pass it this enclosing intermediate completion handler
  // as the completino token.
  //
  // All arguments to our lambda after the first must be defaulted to
  // allow the state machine to be started, as well as to allow the
  // completion handler to match the completion signature of both the
  // async_write and steady_timer::async_wait operations.
  return boost::asio::async_compose<CompletionToken,
                                    void(boost::system::error_code)>(
      [
          // The implementation holds a referene to the socket as it is
          // used for multiple async_write operations.
          &socket,

          // The allocated buffer for the encoding message. The
          // std::unique_ptr smart pointer is move-only, and as a
          // consequence our lambda implementation is also move-only.
          encoded_message = std::move(encoded_message),

          // The repeat count remaining.
          repeat_count,

          // A steady timer used for introducing a delay.
          delay_timer = std::move(delay_timer),

          // To manage the cycle between the multiple underlying
          // asynchronous operation, our implementation is a state
          // machine.
          state = state::starting](auto &self,
                                   const boost::system::error_code &err = {},
                                   std::size_t = 0) mutable {
        if (!err) {
          switch (state) {
          case state::starting:
          case state::writing:
            if (repeat_count > 0) {
              --repeat_count;
              state = state::waiting;
              delay_timer->expires_after(std::chrono::seconds(1));
              delay_timer->async_wait(std::move(self));
              return; // Composed operation not yet complete.
            }
            break; // Composed operation complete, continue below.
          case state::waiting:
            state = state::writing;
            boost::asio::async_write(
                socket, boost::asio::buffer(*encoded_message), std::move(self));
            return; // Composed operation not yet complete.
          }
        }
        // This point is reached only on completion of the entire composed
        // operation.

        // deallocate the encoded message and delay timer before calling
        // the used-supplied completion handler .
        encoded_message.reset();
        delay_timer.reset();

        // Call the user-supplied handler with the result of the
        // operation.
        self.complete(err);
      },
      token, socket);
}

void test_callback() {
  boost::asio::io_context io_context;
  boost::asio::ip::tcp::acceptor acceptor(io_context,
                                          {boost::asio::ip::tcp::v4(), 1234});
  boost::asio::ip::tcp::socket socket = acceptor.accept();

  // Test our asynchronous operation using a lambda as a callback.
  async_write_message(socket, "Testing callback\n", 5,
                      [](const boost::system::error_code &err) {
                        if (!err) {
                          std::cout << "Message sent" << std::endl;
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

  // Test our asynchronous operation using the use_future completion token.
  // This token causes the operation's initiating function to return a future,
  // which may be used to synchronously wait for the result of operation.
  std::future<void> f = async_write_message(socket, "Testing future\n", 5,
                                            boost::asio::use_future);
  io_context.run();
  try {
    f.get();
    std::cout << "Message sent" << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }
}

int main() {
  test_callback();
  test_future();
  return 0;
}
