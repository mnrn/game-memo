#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>
#include <functional>
#include <iostream>
#include <string>

//
// This class manages socket timeouts by applying the concept of a deadline.
// Some asynchronous operations are given deadlines by which they must complete.
// Deadlines are enforced by an "actor" that persists for the lifetime of the
// client object:
//
//  +----------------+
//  |                |
//  | check_deadline |<---+
//  |                |    |
//  +----------------+    | async_wait()
//              |         |
//              +---------+
//
// If the deadline actor determines that the deadline has expired, the socket
// is closed and any outstanding operations are consequently cancelled.
//
// Connection establishment involves trying each endpoint in turn until a
// connection is successful, or the available endpoints are exhausted. If the
// deadline actor closes the socket, the connect actor is woken up and moves to
// the next endpoint.
//
//  +---------------+
//  |               |
//  | start_connect |<---+
//  |               |    |
//  +---------------+    |
//           |           |
//  async_-  |    +----------------+
// connect() |    |                |
//           +--->| handle_connect |
//                |                |
//                +----------------+
//                          :
// Once a connection is     :
// made, the connect        :
// actor forks in two -     :
//                          :
// an actor for reading     :       and an actor for
// inbound messages:        :       sending heartbeats:
//                          :
//  +------------+          :          +-------------+
//  |            |<- - - - -+- - - - ->|             |
//  | start_read |                     | start_write |<---+
//  |            |<---+                |             |    |
//  +------------+    |                +-------------+    | async_wait()
//          |         |                        |          |
//  async_- |    +-------------+       async_- |    +--------------+
//   read_- |    |             |       write() |    |              |
//  until() +--->| handle_read |               +--->| handle_write |
//               |             |                    |              |
//               +-------------+                    +--------------+
//
// The input actor reads messages from the socket, where messages are delimited
// by the newline character. The deadline for a complete message is 30 seconds.
//
// The heartbeat actor sends a heartbeat (a message that consists of a single
// newline character) every 10 seconds. In this example, no deadline is applied
// to message sending.
//
// FROM:
// https://www.boost.org/doc/html/boost_asio/example/cpp11/timeouts/async_tcp_client.cpp
//

class client {
public:
  using tcp = boost::asio::ip::tcp;
  using steady_timer = boost::asio::steady_timer;
  explicit client(boost::asio::io_context &io_context)
      : socket_(io_context), deadline_(io_context),
        heartbeat_timer_(io_context) {}

  // Called by the user of the client class to initiate the connection process.
  // The endpoints will have been obtained using a tcp::resolver.
  void start(tcp::resolver::results_type endpoints) {
    // Start the connect actor.
    endpoints_ = endpoints;
    start_connect(endpoints_.begin());

    // Start the deadline actor. You will note that we're not setting asy
    // particular deadline here. Instead, the connect and input actors will
    // update the deadline prior to each asynchronous operation.
    deadline_.async_wait(std::bind(&client::check_deadline, this));
  }

  // This function terminates all the actor to shut down the connection. It may
  // be called by the user of the client class, or by the class itself in
  // response to graceful termination or an unrecoverable error.
  void stop() {
    stopped_ = true;
    boost::system::error_code ignore_error;
    socket_.close(ignore_error);
    deadline_.cancel();
    heartbeat_timer_.cancel();
  }

private:
  void start_connect(tcp::resolver::results_type::iterator endpoint_iter) {
    if (endpoint_iter != endpoints_.end()) {
      std::cout << "Trying " << endpoint_iter->endpoint() << "...\n";

      // Set a deadline for the connect operation.
      deadline_.expires_after(std::chrono::seconds(60));

      // Start the asynchronous connect operation.
      socket_.async_connect(endpoint_iter->endpoint(),
                            std::bind(&client::handle_connect, this,
                                      std::placeholders::_1, endpoint_iter));
    } else {
      // There are no more endpoints to try. Shut down the client
      stop();
    }
  }

  void handle_connect(const boost::system::error_code &err,
                      tcp::resolver::results_type::iterator endpoint_iter) {
    if (stopped_) {
      return;
    }

    // The async_connect() function automatically opens the socket at hte start
    // of the asynchronous operations. If the socket is  closed at this time
    // then the timeout handler must have run first.
    if (!socket_.is_open()) {
      std::cout << "Connect timed out" << std::endl;
      start_connect(++endpoint_iter);
    }
    // Check if the connect operartion failed before the deadline expired.
    else if (err) {
      std::cout << "Connect error: " << err.message() << std::endl;

      // We need to close the coket used in the previous connection attempt
      // before starting a new one.
      socket_.close();
    }
    // Otherwise we have successfully established a connection.
    else {
      std::cout << "Connected to " << endpoint_iter->endpoint() << std::endl;

      // Start the input actor.
      start_read();

      // Start the heartbeat actor.
      start_write();
    }
  }

  void start_read() {
    // Set a deadlne for the read operation.
    deadline_.expires_after(std::chrono::seconds(30));

    // Start an asynchronous operation to read a newline-delimited message.
    boost::asio::async_read_until(
        socket_, boost::asio::dynamic_buffer(input_buffer_), '\n',
        std::bind(&client::handle_read, this, std::placeholders::_1,
                  std::placeholders::_2));
  }

  void handle_read(const boost::system::error_code &err, std::size_t n) {
    if (stopped_) {
      return;
    }

    if (!err) {
      // Extract the newline-delimited message from the buffer.
      std::string line(input_buffer_.substr(0, n - 1));
      input_buffer_.erase(0, n);

      // Empty messages are heartbeats and so ignored.
      if (!line.empty()) {
        std::cout << "Received: " << line << std::endl;
      }

      start_read();
    } else {
      std::cout << "Error on receive: " << err.message() << std::endl;

      stop();
    }
  }

  void start_write() {
    if (stopped_) {
      return;
    }

    // Start an asynchronous operation to send a heartbeat message.
    boost::asio::async_write(
        socket_, boost::asio::buffer("\n", 1),
        std::bind(&client::handle_write, this, std::placeholders::_1));
  }

  void handle_write(const boost::system::error_code &err) {
    if (stopped_) {
      return;
    }

    if (!err) {
      // Wait 10 seconds before seding the next heartbeat.
      heartbeat_timer_.expires_after(std::chrono::seconds(10));
      heartbeat_timer_.async_wait(std::bind(&client::start_write, this));
    } else {
      std::cout << "Error on heartbeat: " << err.message() << std::endl;

      stop();
    }
  }

  void check_deadline() {
    if (stopped_) {
      return;
    }

    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (deadline_.expiry() <= steady_timer::clock_type::now()) {
      // The deadline has passed. The socket is closed so tha any outstanding
      // asynchoronous operation are cancelled.
      socket_.close();

      // There is no longer an active deadline. The expiry is set to the maximum
      // time point so that the actor takes no action until a new deadline is
      // set.
      deadline_.expires_at(steady_timer::time_point::max());
    }

    // Put the actor back to sleep.
    deadline_.async_wait(std::bind(&client::check_deadline, this));
  }

  bool stopped_ = false;
  tcp::resolver::results_type endpoints_;
  tcp::socket socket_;
  std::string input_buffer_;
  steady_timer deadline_;
  steady_timer heartbeat_timer_;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: client <host> <port>" << std::endl;
      return 1;
    }
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver r(io_context);
    client c(io_context);
    c.start(r.resolve(argv[1], argv[2]));
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Excetion: " << e.what() << std::endl;
  }
}
