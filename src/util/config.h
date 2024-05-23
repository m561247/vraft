#ifndef VRAFT_CONFIG_H_
#define VRAFT_CONFIG_H_

#include <memory>
#include <string>
#include <vector>

#include "cxxopts.hpp"
#include "hostport.h"

namespace vraft {

class Config;
Config &GetConfig();

enum RaftMode {
  kSingleMode = 0,
  kMultiMode = 1,
};

class Config final {
 public:
  Config();
  ~Config();
  Config(const Config &c) = default;
  Config &operator=(const Config &c) = default;

  void Init(int32_t argc, char **argv);
  const std::string Usage();
  const std::string UsageBanner(char *program_name);
  const std::string ProgramName();
  const cxxopts::ParseResult &result() const { return result_; }
  const std::string ToString() const;

  void set_my_addr(HostPort my_addr) { my_addr_ = my_addr; }
  void set_path(std::string path) { path_ = path; }

  const HostPort &my_addr() { return my_addr_; }
  std::vector<HostPort> &peers() { return peers_; }
  uint8_t log_level() const { return log_level_; }
  bool enable_debug() const { return enable_debug_; }
  const std::string &path() { return path_; }
  const RaftMode mode() { return mode_; }

 private:
  std::shared_ptr<cxxopts::Options> options_;
  cxxopts::ParseResult result_;

  HostPort my_addr_;
  std::vector<HostPort> peers_;
  uint8_t log_level_;
  bool enable_debug_;
  std::string path_;
  RaftMode mode_;
};

inline Config::Config() {}

inline Config::~Config() {}

inline const std::string Config::Usage() {
  std::string str;
  if (options_) {
    str = options_->help();
  }
  return str;
}

inline const std::string Config::UsageBanner(char *program_name) {
  std::string str = "Example:\n";
  char buf[256];
  snprintf(buf, sizeof(buf),
           "%s --mode=single --addr=127.0.0.1:9000 "
           "--peers=127.0.0.1:9001,127.0.0.1:9002 "
           "--path=./vraft_9000\n",
           program_name);
  str.append(buf);
  snprintf(buf, sizeof(buf),
           "%s --addr=127.0.0.1:9000 --peers=127.0.0.1:9001,127.0.0.1:9002 "
           "--path=./vraft_9000\n",
           program_name);
  str.append(buf);
  snprintf(buf, sizeof(buf),
           "%s --addr=127.0.0.1:9001 --peers=127.0.0.1:9000,127.0.0.1:9002 "
           "--path=./vraft_9001\n",
           program_name);
  str.append(buf);
  snprintf(buf, sizeof(buf),
           "%s --addr=127.0.0.1:9002 --peers=127.0.0.1:9000,127.0.0.1:9001 "
           "--path=./vraft_9002\n",
           program_name);
  str.append(buf);
  snprintf(buf, sizeof(buf),
           "%s --addr=127.0.0.1:9000 "
           "--peers=127.0.0.1:9001,127.0.0.1:9002 "
           "--path=./raft_insight_test\n",
           program_name);
  str.append(buf);
  snprintf(buf, sizeof(buf), "%s -h\n", program_name);
  str.append(buf);
  snprintf(buf, sizeof(buf), "%s --help\n", program_name);
  str.append(buf);
  return str;
}

inline const std::string Config::ProgramName() { return options_->program(); }

inline const std::string Config::ToString() const {
  std::string str;
  char buf[512];
  snprintf(buf, sizeof(buf), "my_addr: %s\n", my_addr_.ToString().c_str());
  str.append(buf);
  str.append("peers:\n");
  for (auto &o : peers_) {
    snprintf(buf, sizeof(buf), "\t%s\n", o.ToString().c_str());
    str.append(buf);
  }
  snprintf(buf, sizeof(buf), "path: %s\n", path_.c_str());
  str.append(buf);

  return str;
}

}  // namespace vraft

#endif
