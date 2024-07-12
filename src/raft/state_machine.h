#ifndef VRAFT_STATE_MACHINE_H_
#define VRAFT_STATE_MACHINE_H_

#include <cstdint>
#include <memory>

#include "common.h"
#include "raft_addr.h"

namespace vraft {

using CreateSMFunc = std::function<StateMachineSPtr(std::string &path)>;

class LogEntry;

class SnapshotReader {
 public:
  SnapshotReader(std::string path) : path_(path) {}
  virtual ~SnapshotReader() {}
  SnapshotReader(const SnapshotReader &) = delete;
  SnapshotReader &operator=(const SnapshotReader &) = delete;

  std::string path() { return path_; }
  RaftIndex last_index() { return last_index_; }
  RaftTerm last_term() { return last_term_; }

  virtual int32_t Start() = 0;
  virtual int32_t Read(std::string &data, int32_t &offset) = 0;
  virtual int32_t Finish() = 0;

 protected:
  std::string path_;
  RaftIndex last_index_;
  RaftTerm last_term_;
};

class SnapshotWriter {
 public:
  SnapshotWriter(std::string path, RaftIndex last_index, RaftTerm last_term)
      : path_(path), last_index_(last_index), last_term_(last_term) {}
  virtual ~SnapshotWriter() {}
  SnapshotWriter(const SnapshotWriter &) = delete;
  SnapshotWriter &operator=(const SnapshotWriter &) = delete;

  std::string path() { return path_; }
  RaftIndex last_index() { return last_index_; }
  RaftTerm last_term() { return last_term_; }

  virtual int32_t Start() = 0;
  virtual int32_t Write(std::string &data, int32_t &offset) = 0;
  virtual int32_t Finish() = 0;

 protected:
  std::string path_;
  RaftIndex last_index_;
  RaftTerm last_term_;
};

class StateMachine {
 public:
  StateMachine(std::string path) : path_(path) {}
  virtual ~StateMachine() {}
  StateMachine(const StateMachine &t) = delete;
  StateMachine &operator=(const StateMachine &t) = delete;

  virtual int32_t Restore() = 0;
  virtual int32_t Apply(LogEntry *entry, RaftAddr addr) = 0;
  virtual RaftIndex LastIndex() = 0;
  virtual RaftTerm LastTerm() = 0;

  std::string path() { return path_; }

 public:
  SnapshotReaderSPtr reader_;
  SnapshotWriterSPtr writer_;

 protected:
  std::string path_;
};

}  // namespace vraft

#endif
