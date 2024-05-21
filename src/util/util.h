#ifndef VRAFT_UTIL_H_
#define VRAFT_UTIL_H_

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cassert>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace vraft {

void Split(const std::string &str, char separator,
           std::vector<std::string> &result);

void DelSpace(const std::string &str);

std::string CurrentTidStr();

void ToLower(std::string &input);

// ip_out是主机字节序，小端
bool HostNameToIpU32(const std::string &hostname, uint32_t &ip_out);

// ip_out是主机字节序，小端
bool IpStringToIpU32(const std::string &ip_str, uint32_t &ip_out);

// ip_out是主机字节序，小端
bool StringToIpU32(const std::string &str, uint32_t &ip_out);

// ip是主机字节序，小端
std::string IpU32ToIpString(uint32_t ip);

uint32_t Crc32(const void *data, size_t length);

std::string StrToHexStr(const char *ptr, int32_t size);

std::string PointerToHexStr(void *p);

std::string U32ToHexStr(uint32_t x);

std::string NsToString(uint64_t ns);

}  // namespace vraft

#endif
