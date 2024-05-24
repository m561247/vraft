#include <gtest/gtest.h>

#include <cstdlib>
#include <iostream>

#include "coding.h"
#include "raft_log.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << argv[0] << " db_parth" << std::endl;
    return 0;
  }

  std::string path = argv[1];

  leveldb::Options o;
  o.create_if_missing = true;
  o.error_if_exists = false;
  o.comparator = vraft::U32Comparator();
  leveldb::DB *db;
  leveldb::Status status = leveldb::DB::Open(o, path, &db);
  assert(status.ok());

  leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::string key_str = it->key().ToString();
    assert(key_str.size() == sizeof(uint32_t));
    uint32_t key32 = vraft::DecodeFixed32(key_str.c_str());
    std::string tmp_key;
    for (auto c : key_str) {
      char buf[2];
      snprintf(buf, sizeof(buf), "%X", c);
      tmp_key.append(buf);
    }

    std::string tmp_value;
    std::string value_str = it->value().ToString();
    if (value_str.size() == sizeof(uint64_t)) {
      /*
      for (auto c : value_str) {
        char buf[2];
        snprintf(buf, sizeof(buf), "%X", c);
        tmp_value.append(buf);
      }
      */

      uint64_t term = vraft::DecodeFixed64(value_str.c_str());
      char buf[128];
      snprintf(buf, sizeof(buf), "%lu", term);
      tmp_value.append(buf);
    } else {
      tmp_value = value_str;
    }

    std::cout << key32 << " " << tmp_key << " : " << tmp_value << std::endl;
  }
  assert(it->status().ok());  // Check for any errors found during the scan
  delete it;
  delete db;

  return 0;
}