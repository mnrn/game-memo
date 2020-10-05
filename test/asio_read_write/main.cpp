#include <array>
#include <boost/asio.hpp>
#include <iostream>
#include <memory>

namespace third_party_lib {

class session {
public:
  using tcp = boost::asio::ip::tcp;
  using error_code = boost::system::error_code;

  enum struct state {
    reading,
    writing,
  };

  explicit session(tcp::socket &socket) : socket_(socket) {}

  // Returns true if the third party library wawnts to be notified when the
  // socket is ready for reading.
  constexpr bool want_read() const { return state_ == state::reading; }

  // Notify that third party library that it should perform its read operation.
  void do_read(error_code &err) {
    if (std::size_t len = socket_.read_some(boost::asio::buffer(data_), err)) {
      write_buffer_ = boost::asio::buffer(data_, len);
      state_ = state::writing;
    }
  }

  // Returns true if the third party library wants to be notified when the
  // socket is ready for writing.
  constexpr bool want_write() const { return state_ == state::writing; }

  // Notify that third party library that it should perform its write operation.
  void do_write(error_code &err) {
    if (std::size_t len =
            socket_.write_some(boost::asio::buffer(write_buffer_), err)) {
      write_buffer_ = write_buffer_ + len;
      state_ = boost::asio::buffer_size(write_buffer_) > 0 ? state::writing
                                                           : state::reading;
    }
  }

private:
  tcp::socket &socket_;
  state state_ = state::reading;
  std::array<char, 128> data_;
  boost::asio::const_buffer write_buffer_;
};

} // namespace third_party_lib

// The Glue between asio's sockets and third party library.
class connection : public std::enable_shared_from_this<connection> {
public:
  using tcp = boost::asio::ip::tcp;
  using error_code = boost::system::error_code;

  explicit connection(tcp::socket socket) : socket_(std::move(socket)) {}

  void start() {
    // Put the socket into non-blocking mode.
    socket_.non_blocking(true);

    do_operations();
  }

private:
  void do_operations() {
    auto self(shared_from_this());

    // Start a read opearation if the third party library wants one.
    if (session_.want_read() && !read_in_progress_) {
      read_in_progress_ = true;
      socket_.async_wait(tcp::socket::wait_read, [this, self](error_code err) {
        read_in_progress_ = false;

        // Notify third party library that it can perform a read.
        if (!err) {
          session_.do_read(err);

          // The third party library successfully performed a read on the
          // socket. Start new read or write operations based on what it now
          // wants.
          if (!err || err == boost::asio::error::would_block) {
            do_operations();
          }
          // Otherwise, an error occurred, Closing the socket cancels any
          // outstating asynchronous read or write operations. The connecton
          // object will be destroyed automatically once those outstanding
          // operations complete.
          else {
            socket_.close();
          }
        }
      });
    }

    // Start a write operation if the third party library wants one.
    if (session_.want_write() && !write_in_progress_) {
      write_in_progress_ = true;
      socket_.async_wait(tcp::socket::wait_write, [this, self](error_code err) {
        write_in_progress_ = false;

        // Notify third party successfully performed a write
        // on the socket. Start new read or write operations
        // based on what it now wants.
        if (!err) {
          session_.do_write(err);
        }

        // The third party library successfully performed a
        // write on the socket. Start new read or write
        // operations base on what it now wants.
        if (!err || err == boost::asio::error::would_block) {
          do_operations();
        }
        // Otherwise, an error occurred. Closing the socket
        // cancels any outstanding asynchronous read or write
        // operations. The connection object will be
        // destroyed automatically once those outstatnding
        // operations complete.
        else {
          socket_.close();
        }
      });
    }
  }

  tcp::socket socket_;
  third_party_lib::session session_{socket_};
  bool read_in_progress_ = false;
  bool write_in_progress_ = false;
};

class server {
public:
  using tcp = boost::asio::ip::tcp;
  using error_code = boost::system::error_code;

  server(boost::asio::io_context &io_context, std::uint16_t port)
      : acceptor_(io_context, {tcp::v4(), port}) {
    do_accept();
  }

private:
  void do_accept() {
    acceptor_.async_accept([this](error_code err, tcp::socket socket) {
      if (!err) {
        std::make_shared<connection>(std::move(socket))->start();

        do_accept();
      }
    });
  }

  tcp::acceptor acceptor_;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: polled_read_or_write <port>" << std::endl;
      return 1;
    }
    boost::asio::io_context io_context;
    server s(io_context, std::atoi(argv[1]));
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}
