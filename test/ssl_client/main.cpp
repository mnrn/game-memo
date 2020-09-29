#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>

class ssl_client {
public:
  using tcp = boost::asio::ip::tcp;

  ssl_client(boost::asio::io_context &io_context,
             boost::asio::ssl::context &context,
             tcp::resolver::results_type endpoints)
      : socket_(io_context, context) {
    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback(std::bind(&ssl_client::verify_certificate, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2));

    connect(endpoints);
  }

private:
  bool verify_certificate(bool preverified,
                          boost::asio::ssl::verify_context &ctx) {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once for
    // eache certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the cerificate's subject name.
    std::array<char, 256> subject_name{};
    X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name.data(),
                      subject_name.size());
    std::cout << "Verifying "
              << std::string(subject_name.cbegin(), subject_name.cend())
              << std::endl;
    return preverified;
  }

  void connect(const tcp::resolver::results_type &endpoints) {
    boost::asio::async_connect(
        socket_.lowest_layer(), endpoints,
        [this](const boost::system::error_code &err, const tcp::endpoint &) {
          if (!err) {
            handshake();
          } else {
            std::cout << "Connect failed: " << err.message() << std::endl;
          }
        });
  }

  void handshake() {
    socket_.async_handshake(boost::asio::ssl::stream_base::client,
                            [this](const boost::system::error_code &err) {
                              if (!err) {
                                send_request();
                              } else {
                                std::cout
                                    << "Handshake failed: " << err.message()
                                    << std::endl;
                              }
                            });
  }

  void send_request() {
    std::cout << "Enter message: ";
    std::cin.getline(request_.data(), max_length);
    std::size_t req_len = std::strlen(request_.data());

    boost::asio::async_write(
        socket_, boost::asio::buffer(request_, req_len),
        [this](const boost::system::error_code &err, std::size_t len) {
          if (!err) {
            receive_response(len);
          } else {
            std::cout << "Write failed: " << err.message() << std::endl;
          }
        });
  }

  void receive_response(std::size_t len) {
    boost::asio::async_read(
        socket_, boost::asio::buffer(reply_.data(), len),
        [this](const boost::system::error_code &err, std::size_t len) {
          if (!err) {
            std::cout << "Reply: ";
            std::cout.write(reply_.data(), len);
            std::cout << std::endl;
          } else {
            std::cout << "Read failed: " << err.message() << std::endl;
          }
        });
  }

  static inline constexpr std::size_t max_length = 1024;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
  std::array<char, max_length> request_;
  std::array<char, max_length> reply_;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "USage: client <host> <port>" << std::endl;
      return 1;
    }
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver(io_context);
    auto enspoints = resolver.resolve(argv[1], argv[2]);

    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
    ctx.load_verify_file("ca.pem");

    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}
