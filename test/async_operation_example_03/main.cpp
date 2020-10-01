#include <boost/asio/bind_executor.hpp>
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
                       const char *message) {
    // The async_write operatoin has a completion handler signature of:
    //
    //   void(boost::system::error_code& error, std::size_t n)
    //
    // This differs from our operation's signature in that it is also passed the
    // number of bytes tranferred as an argument of type std::size_t. We will
    // adapt our completion handler to async_write's completion handler
    // signature by using std::bind, which drops the additional argument.
    //
    // However, it is essential to the correctness of our composed operation
    // that we preserve the executor of the user-supplied completion handler.
    // The std::bind function will not do this for us, so we must do this by
    // first obtaining the completion handler's associated executor (defaulting
    // completion handler does not have its own)...
    auto executor = boost::asio::get_associated_executor(completion_handler,
                                                         socket.get_executor());
    // ... and then binding this executor to our adapted compltion handler using
    // the boost::asio::binding_executor function.
    boost::asio::async_write(
        socket, boost::asio::buffer(message, std::strlen(message)),
        boost::asio::bind_executor(
            executor, std::bind(std::forward<decltype(completion_handler)>(
                                    completion_handler),
                                std::placeholders::_1)));
  };

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
      initiation, token, std::ref(socket), message);
}

void test_callback() {
  boost::asio::io_context io_context;
  boost::asio::ip::tcp::acceptor acceptor(io_context,
                                          {boost::asio::ip::tcp::v4(), 1234});
  boost::asio::ip::tcp::socket socket = acceptor.accept();

  // Test our asynchronous operation using a lambda as a callback.
  async_write_message(socket, "Testing callback\n",
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
      async_write_message(socket, "Testing future\n", boost::asio::use_future);
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
