#include "util.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

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
  char buf[2];
  memset(buf, 0, sizeof(buf));
  for (int32_t i = 0; i < size; ++i) {
    snprintf(buf, sizeof(buf), "%X", ptr[i]);
    str.append(buf);
  }
  return str;
}

}  // namespace vraft
