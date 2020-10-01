#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
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
                         CompletionToken &&token) {
  // In addition to determining the mechanism by which an asynchronous
  // operationdelivers its result, a completion token also determines the time
  // when the operation commences. For example, when the completion token is a
  // simple callback the operation commences before the initiating function
  // returns. However, if the completion toke's delivery machanism uses a
  // future, we might instead want to defer initiation of the operation until
  // the returned future object is waited upon.
  //
  // To enable this, when implementing an asynchronous operation we must package
  // the initiation step as a function object. The initiation function object's
  // call operation is passed the concrete completion handler produced by the
  // completion token. This completion handler matches the asynchoronous
  // operation's completion handler signature which in this example is:
  //
  //   void(boost::system::error_code error)
  //
  // The initiation function object also receives any additional argmuments
  // required to start the operation. (Note: We could have instead passed these
  // arguments in the lambda capture set. However, we should prefer to propagate
  // them as function call arguments as this allows the completion token to
  // optimise ho they are passed. For example, a lazy future which defers
  // initiation would need to make a decay-copy of the arguments, but when using
  // a simple callback the arguments can be trivially forwarded straight
  // through.)
  auto initiation = [](auto &&completion_handler,
                       boost::asio::ip::tcp::socket &socket,
                       std::unique_ptr<std::string> encoded_message) {
    // In this example, the composed operation's intermediate completion handler
    // is implemented as a hand-crafted function object, rather than using a
    // lambda or std::bind.
    struct intermediate_completion_handler {
      // The intermediate completion handler holds a reference o the socket so
      // that it can obtaion the I/O executor (see get_executor below).
      boost::asio::ip::tcp::socket &socket_;

      // The allocated buffer for the encoded message. The std::unique_ptr smart
      // pointer is move-only, and as a consequence our intermediate completion
      // handler is also move-only.
      std::unique_ptr<std::string> encoded_message_;

      // The user-supplied completion handler.
      typename std::decay_t<decltype(completion_handler)> handler_;

      // The function call operator matches the completion signature of the
      // async_write operation.
      void operator()(const boost::system::error_code &err, std::size_t) {
        // Deallocate the encoded message before calling the user-supplied
        // completion handler.
        encoded_message_.reset();

        // Call the user-supplied handler with the result of the operation. The
        // arguments must match the completion signature of our composed
        // operation.
        handler_(err);
      }

      // It is essential to the correctness of our composed operation
      // that we preserve the executor of the user-supplied completion handler.
      // The std::bind function will not do this for us, so we must do this by
      // first obtaining the completion handler's associated executor and
      // defaulting to the I/O executor - in this case the executor of the
      // socket
      // - if the completion handler does not have its own.
      using executor_type = boost::asio::associated_executor_t<
          typename std::decay_t<decltype(completion_handler)>,
          boost::asio::ip::tcp::socket::executor_type>;

      executor_type get_executor() const noexcept {
        return boost::asio::get_associated_executor(handler_,
                                                    socket_.get_executor());
      }

      // Althrough not neccessary for correctness, we may also presere the
      // allocator of the user-supplied completion handler. This is archieved by
      // defining a nested type allocator_type and member function
      // get_allocator. These obtain the completion handler's associated
      // allocator, and default to std::allocator<void> if the completion
      // handler does not have its own.
      using allocator_type = boost::asio::associated_allocator_t<
          typename std::decay_t<decltype(completion_handler)>,
          std::allocator<void>>;

      allocator_type get_allocator() const noexcept {
        return boost::asio::get_associated_allocator(handler_,
                                                     std::allocator<void>{});
      }
    };

    // Initiate the underlying async_write operaton using our intermediate
    // completion handler.
    auto encoded_message_buffer = boost::asio::buffer(*encoded_message);
    boost::asio::async_write(
        socket, encoded_message_buffer,
        intermediate_completion_handler{
            socket, std::move(encoded_message),
            std::forward<decltype(completion_handler)>(completion_handler)});
  };

  // Encoded the message and copy it into an allocated buffer. The buffer will
  // be maintained for the lifetime of the asynchornous operation.
  std::ostringstream os;
  os << message;
  std::unique_ptr<std::string> encoded_message =
      std::make_unique<std::string>(os.str());

  // The boost::asio::async_initiate function takes:
  //
  // - our initiation function object,
  // - the completion token,
  // - the completion handler signature, and
  // - any additional arguments we need to initate the operation.
  //
  // It then asks the completion token to create a completion handler (i.e. a
  // callback) with the specified signature, and invoke the initiation function
  // object with this completion handler as well as the additional arguments.
  // The return value of async_initiate is the result of our opearation's
  // initiating function.
  //
  // Note that we drop wrap non-const reference arguments in
  // std::reference_wrapper to prevent incorrect decay-copies of these objects.
  return boost::asio::async_initiate<CompletionToken,
                                     void(boost::system::error_code)>(
      initiation, token, std::ref(socket), std::move(encoded_message));
}

void test_callback() {
  boost::asio::io_context io_context;
  boost::asio::ip::tcp::acceptor acceptor(io_context,
                                          {boost::asio::ip::tcp::v4(), 1234});
  boost::asio::ip::tcp::socket socket = acceptor.accept();

  // Test our asynchronous operation using a lambda as a callback.
  async_write_message(socket, 1234456,
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

  // Test our asynchronous operation using the use_future completion token. This
  // token causes the operation's initiating function to return a future, which
  // may be used to synchronously wait for the result of operation.
  std::future<void> f =
      async_write_message(socket, 654.321, boost::asio::use_future);
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
