#include "timer_manager.h"

namespace vraft {

void TimerManager::StartTick() { tick_->Start(); }

void TimerManager::AgainElection() {
  election_->Again(random_election_ms_.Get(), 0);
}

void TimerManager::AgainElectionRpc() {
  for (auto &item : request_votes_) {
    if (item.second) {
      item.second->Again();
    }
  }
}

void TimerManager::AgainElectionRpc(uint64_t addr) {
  auto it = request_votes_.find(addr);
  if (it != request_votes_.end()) {
    if (it->second) {
      it->second->Again();
    }
  }
}

void TimerManager::AgainHeartBeat() {
  for (auto &item : heartbeats_) {
    if (item.second) {
      item.second->Start();
    }
  }
}

void TimerManager::AgainHeartBeat(uint64_t addr) {
  auto it = heartbeats_.find(addr);
  if (it != heartbeats_.end()) {
    if (it->second) {
      it->second->Start();
    }
  }
}

void TimerManager::Stop() {
  StopTick();
  StopElection();
  StopElectionRpc();
  StopHeartBeat();
}

void TimerManager::StopTick() { tick_->Stop(); }

void TimerManager::StopElection() { election_->Stop(); }

void TimerManager::StopElectionRpc() {
  for (auto &item : request_votes_) {
    if (item.second) {
      item.second->Stop();
    }
  }
}

void TimerManager::StopElectionRpc(uint64_t addr) {
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
