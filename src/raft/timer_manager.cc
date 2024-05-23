#include "timer_manager.h"

namespace vraft {

void TimerManager::MakeTimer() {
  MakeTick();
  MakeElection();
  MakeElectionPpc();
  MakeHeartbeat();
}

void TimerManager::MakeTick() {
  assert(maketimer_func_);
  tick_ = maketimer_func_(0, tick_ms_, tick_func_, data_);
}

void TimerManager::MakeElection() {
  assert(maketimer_func_);
  election_ =
      maketimer_func_(random_election_ms_.Get(), 0, election_func_, data_);
}

void TimerManager::MakeElectionPpc() {
  assert(maketimer_func_);
  for (auto &item : request_votes_) {
    item.second =
        maketimer_func_(0, request_vote_ms_, requestvote_func_, data_);
    item.second->set_dest_addr(item.first);
  }
}

void TimerManager::MakeHeartbeat() {
  assert(maketimer_func_);
  for (auto &item : heartbeats_) {
    item.second = maketimer_func_(0, heartbeat_ms_, heartbeat_func_, data_);
    item.second->set_dest_addr(item.first);
  }
}

void TimerManager::StartTick() { tick_->Start(); }

void TimerManager::StartElection() {
  election_->Again(0, random_election_ms_.Get());
}

void TimerManager::AgainElection() {
  election_->Again(random_election_ms_.Get(), 0);
}

void TimerManager::StartRequestVote() {
  for (auto &item : request_votes_) {
    if (item.second) {
      item.second->Start();
    }
  }
}

void TimerManager::StartRequestVote(uint64_t addr) {
  auto it = request_votes_.find(addr);
  if (it != request_votes_.end()) {
    if (it->second) {
      it->second->Start();
    }
  }
}

void TimerManager::StartHeartBeat() {
  for (auto &item : heartbeats_) {
    if (item.second) {
      item.second->Again(0, heartbeat_ms_);
    }
  }
}

void TimerManager::StartHeartBeat(uint64_t addr) {
  auto it = heartbeats_.find(addr);
  if (it != heartbeats_.end()) {
    if (it->second) {
      it->second->Again(0, heartbeat_ms_);
    }
  }
}

void TimerManager::AgainHeartBeat() {
  for (auto &item : heartbeats_) {
    if (item.second) {
      item.second->Again(heartbeat_ms_, heartbeat_ms_);
    }
  }
}

void TimerManager::AgainHeartBeat(uint64_t addr) {
  auto it = heartbeats_.find(addr);
  if (it != heartbeats_.end()) {
    if (it->second) {
      it->second->Again(heartbeat_ms_, heartbeat_ms_);
    }
  }
}

void TimerManager::Stop() {
  StopTick();
  StopElection();
  StopRequestVote();
  StopHeartBeat();
}

void TimerManager::StopTick() { tick_->Stop(); }

void TimerManager::StopElection() { election_->Stop(); }

void TimerManager::StopRequestVote() {
  for (auto &item : request_votes_) {
    if (item.second) {
      item.second->Stop();
    }
  }
}

void TimerManager::StopRequestVote(uint64_t addr) {
  auto it = request_votes_.find(addr);
  if (it != request_votes_.end()) {
    if (it->second) {
      it->second->Stop();
    }
  }
}

void TimerManager::StopHeartBeat() {
  for (auto &item : heartbeats_) {
    if (item.second) {
      item.second->Stop();
    }
  }
}

void TimerManager::StopHeartBeat(uint64_t addr) {
  auto it = heartbeats_.find(addr);
  if (it != heartbeats_.end()) {
    if (it->second) {
      it->second->Stop();
    }
  }
}

}  // namespace vraft
