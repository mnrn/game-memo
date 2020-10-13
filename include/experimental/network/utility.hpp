#ifndef EXPERIMENTAL_NETWORK_UTILITY_HPP
#define EXPERIMENTAL_NETWORK_UTILITY_HPP

#include <memory>
#include <set>
#include <string>

namespace net {
class subscriber {
public:
  virtual ~subscriber() = default;
  virtual void deliver(const std::string &msg) = 0;
};

class channel {
public:
  void join(std::shared_ptr<subscriber> subscriber) {
    subscribers_.emplace(subscriber);
  }
  void leave(std::shared_ptr<subscriber> subscriber) {
    subscribers_.erase(subscriber);
  }
  void deliver(const std::string &msg) {
    for (const auto &s : subscribers_) {
      s->deliver(msg);
    }
  }

private:
  std::set<std::shared_ptr<subscriber>> subscribers_;
};

} // namespace net

#endif
