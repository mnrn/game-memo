#ifndef HTTP_PATH_HPP
#define HTTP_PATH_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <string>

namespace net {
namespace http {
namespace url {

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
static inline std::string path_cat(boost::beast::string_view base,
                                   boost::beast::string_view path) {
  if (base.empty()) {
    return std::string(path);
  }
  std::string result(base);
#ifdef BOOST_MSVC
  constexpr char path_sep = '\\';
  if (result.back() == path.sep) {
    result.resize(result.size() - 1);
  }
  result.append(path.data(), path.size());
  for (auto &c : result) {
    if (c == '/') {
      c = path_sep;
    }
  }
#else
  constexpr char path_sep = '/';
  if (result.back() == path_sep) {
    result.resize(result.size() - 1);
  }
  result.append(path.data(), path.size());
#endif
  return result;
}

} // namespace url
} // namespace http
} // namespace net

#endif
