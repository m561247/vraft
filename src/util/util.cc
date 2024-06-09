#include "util.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

#include "clock.h"

namespace vraft {

void Split(const std::string &str, char separator,
           std::vector<std::string> &result) {
  size_t start = 0;
  size_t end = str.find(separator);

  while (end != std::string::npos) {
    result.push_back(str.substr(start, end - start));
    start = end + 1;
    end = str.find(separator, start);
  }

  result.push_back(str.substr(start));
}

void DelSpace(std::string &str) {
  std::string result;
  for (char c : str) {
    if (c != ' ' && c != '\t') {
      result += c;
    }
  }
  str = result;
}

std::string CurrentTidStr() {
  std::thread::id tid = std::this_thread::get_id();
  std::ostringstream oss;
  oss << tid;
  return oss.str();
}

void ToLower(std::string &input) {
  std::transform(input.begin(), input.end(), input.begin(),
                 [](unsigned char c) -> char { return std::tolower(c); });
}

bool HostNameToIpU32(const std::string &hostname, uint32_t &ip_out) {
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;  // 限定为IPv4

  if (getaddrinfo(hostname.c_str(), nullptr, &hints, &res) != 0) {
    return false;  // 解析域名失败
  }

  for (struct addrinfo *p = res; p != nullptr; p = p->ai_next) {
    // 尝试找到第一个IPv4地址
    if (p->ai_family == AF_INET) {
      struct sockaddr_in *ipv4 =
          reinterpret_cast<struct sockaddr_in *>(p->ai_addr);
      ip_out = ntohl(ipv4->sin_addr.s_addr);  // 将网络字节序转换为主机字节序
      freeaddrinfo(res);
      return true;  // 成功转换
    }
  }

  freeaddrinfo(res);
  return false;  // 未找到IPv4地址
}

bool IpStringToIpU32(const std::string &ip_str, uint32_t &ip_out) {
  struct in_addr addr;
  // inet_pton函数将点分十进制的字符串IP地址转换为网络字节序的二进制形式
  int result = inet_pton(AF_INET, ip_str.c_str(), &addr);
  if (result <= 0) {
    if (result == 0)
      std::cerr << "Not in presentation format" << std::endl;
    else
      perror("inet_pton");
    return false;
  }

  ip_out = ntohl(addr.s_addr);  // 网络字节序转换为主机字节序
  return true;
}

bool StringToIpU32(const std::string &str, uint32_t &ip_out) {
  bool b = IpStringToIpU32(str, ip_out);
  if (b) {
    // std::cout << "IpStringToIpU32 ok" << std::endl;
    return b;
  }

  b = HostNameToIpU32(str, ip_out);
  if (b) {
    // std::cout << "HostNameToIpU32 ok" << std::endl;
    return b;
  }

  assert(!b);
  return b;
}

std::string IpU32ToIpString(uint32_t ip) {
  // 使用inet_ntoa进行转换，先转换为网络字节序
  in_addr addr;
  addr.s_addr = htonl(ip);  // 主机字节序转为网络字节序
  // inet_ntoa返回点分十进制形式的字符串
  const char *ip_str = inet_ntoa(addr);
  if (ip_str != nullptr) {
    return std::string(ip_str);
  }
  // 若转换失败，返回空字符串或预定义错误提示
  return "";
}

uint32_t Crc32(const void *data, size_t length) {
  uint32_t crc = 0xFFFFFFFF;
  const uint8_t *buf = static_cast<const uint8_t *>(data);

  for (size_t i = 0; i < length; ++i) {
    crc ^= buf[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320;
      } else {
        crc = crc >> 1;
      }
    }
  }

  return ~crc;
}

std::string StrToHexStr(const char *ptr, int32_t size) {
  assert(size >= 0);
  std::string str;
  char buf[8];
  memset(buf, 0, sizeof(buf));
  for (int32_t i = 0; i < size; ++i) {
    snprintf(buf, sizeof(buf), "0x%X ", ptr[i]);
    str.append(buf);
  }
  if (str.size() > 0) {
    str.pop_back();
  }

  return str;
}

std::string PointerToHexStr(void *p) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%p", p);
  return std::string(buf);
}

std::string U32ToHexStr(uint32_t x) {
  char buf[32];
  snprintf(buf, sizeof(buf), "0x%X", x);
  return std::string(buf);
}

std::string NsToString(uint64_t ns) {
  // Convert ns to system time point
  std::chrono::nanoseconds nano(ns);
  std::chrono::system_clock::time_point time_point_ns =
      std::chrono::system_clock::time_point(
          std::chrono::duration_cast<std::chrono::system_clock::duration>(
              nano));

  // Convert time point to time_t, and then to tm for easy access of year,
  // month, day, etc. system_clock::to_time_t() converts time point to time_t by
  // truncating the fractional seconds
  std::time_t time_t_ns = std::chrono::system_clock::to_time_t(time_point_ns);
  std::tm tm = *std::localtime(&time_t_ns);

  // Extract the nanosecond part by considering the truncation to seconds
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(nano);
  uint64_t total_ns = nano.count();
  uint64_t leftover_ns =
      total_ns -
      std::chrono::duration_cast<std::chrono::nanoseconds>(seconds).count();

  // Stringstream to format strings with leading zeros & concatenation
  std::ostringstream ss;
  ss << std::put_time(&tm, "%Y:%m:%d %H:%M:%S")
     << " ";  // format and add date and time up to seconds
  ss << std::setw(9) << std::setfill('0')
     << leftover_ns;  // format nanoseconds with padding

  return ss.str();
}

std::string NsToString2(uint64_t ns) {
  // Convert ns to system time point
  std::chrono::nanoseconds nano(ns);
  std::chrono::system_clock::time_point time_point_ns =
      std::chrono::system_clock::time_point(
          std::chrono::duration_cast<std::chrono::system_clock::duration>(
              nano));

  // Convert time point to time_t, and then to tm for easy access of year,
  // month, day, etc. system_clock::to_time_t() converts time point to time_t by
  // truncating the fractional seconds
  std::time_t time_t_ns = std::chrono::system_clock::to_time_t(time_point_ns);
  std::tm tm = *std::localtime(&time_t_ns);

  // Extract the nanosecond part by considering the truncation to seconds
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(nano);
  uint64_t total_ns = nano.count();
  uint64_t leftover_ns =
      total_ns -
      std::chrono::duration_cast<std::chrono::nanoseconds>(seconds).count();

  // Stringstream to format strings with leading zeros & concatenation
  std::ostringstream ss;
  ss << std::put_time(&tm, "%Y:%m:%d-%H:%M:%S")
     << "-";  // format and add date and time up to seconds
  ss << std::setw(9) << std::setfill('0')
     << leftover_ns;  // format nanoseconds with padding

  return ss.str();
}

bool IsNumber(const std::string &str) {
  for (char ch : str) {
    if (!std::isdigit(ch)) {
      return false;
    }
  }
  return true;
}

std::string TidToStr(std::thread::id tid) {
  std::ostringstream oss;
  oss << tid;
  return oss.str();
}

std::string CurrentTid() { return TidToStr(std::this_thread::get_id()); }

bool TidValid(std::thread::id tid) {
  std::string s = TidToStr(tid);
  if (s.size() == 0) return false;
  if (!IsNumber(s)) return false;
  return true;
}

uint32_t UniqId(void *ptr) {
  char buf[sizeof(void *) + sizeof(uint64_t)];
  uint64_t ts = Clock::NSec();
  memcpy(buf, ptr, sizeof(void *));
  char *p = buf + sizeof(void *);
  memcpy(p, &ts, sizeof(uint64_t));
  uint32_t crc32 = Crc32(buf, sizeof(void *) + sizeof(uint64_t));
  return crc32;
}

}  // namespace vraft
