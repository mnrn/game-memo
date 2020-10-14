#ifndef MIME_HPP
#define MIME_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace net {
namespace http {
namespace mime {

// Return a reasonable mime type based on the extension of a file.
static inline boost::beast::string_view type(boost::beast::string_view path) {
  using namespace boost;
  using beast::iequals;
  const auto ext = [&path] {
    const auto pos = path.rfind(".");
    return pos == beast::string_view::npos ? beast::string_view{}
                                           : path.substr(pos);
  }();

  if (iequals(ext, ".htm")) {
    return "text/html";
  } else if (iequals(ext, ".html")) {
    return "text/html";
  } else if (iequals(ext, ".php")) {
    return "text/html";
  } else if (iequals(ext, ".css")) {
    return "text/css";
  } else if (iequals(ext, ".txt")) {
    return "text/plain";
  } else if (iequals(ext, ".js")) {
    return "application/javascript";
  } else if (iequals(ext, ".json")) {
    return "application/json";
  } else if (iequals(ext, ".xml")) {
    return "application/xml";
  } else if (iequals(ext, ".swf")) {
    return "application/x-shockwave-flash";
  } else if (iequals(ext, ".flv")) {
    return "video/x-flv";
  } else if (iequals(ext, ".png")) {
    return "image/png";
  } else if (iequals(ext, ".jpe")) {
    return "image/jpeg";
  } else if (iequals(ext, ".jpeg")) {
    return "image/jpeg";
  } else if (iequals(ext, ".jpg")) {
    return "image/jpeg";
  } else if (iequals(ext, ".gif")) {
    return "image/gif";
  } else if (iequals(ext, ".bmp")) {
    return "image/bmp";
  } else if (iequals(ext, ".ico")) {
    return "image/vnd.microsoft.icon";
  } else if (iequals(ext, ".tiff")) {
    return "image/tiff";
  } else if (iequals(ext, ".tif")) {
    return "image/tiff";
  } else if (iequals(ext, ".svg")) {
    return "image/svg+xml";
  } else if (iequals(ext, ".svgz")) {
    return "image/svg+xml";
  } else {
    return "application/text";
  }
}

} // namespace mime
} // namespace http
} // namespace net

#endif
