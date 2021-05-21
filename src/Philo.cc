//
// Created by Aaron Berry on 5/19/21.
//

#include <iostream>
#include <thread>
#include "includes/Philo.h"
#include "includes/Config.h"

Philo::Philo(Philo &&other) noexcept {
  std::swap(left_fork_, other.left_fork_);
  std::swap(right_fork_, other.right_fork_);
}

Philo &Philo::operator=(Philo &&other) noexcept {
  if (this == &other) {
    return *this;
  }
  left_fork_ = right_fork_ = nullptr;
  std::swap(left_fork_, other.left_fork_);
  std::swap(right_fork_, other.right_fork_);
  return *this;
}

void Philo::SaySomething(const std::string &&str) const noexcept {
  std::lock_guard<std::mutex> lock(config_->GetIoMutex());
  std::cout << "["<< config_->GetTimer().GetTimeSimulation() << "]"
            << ' ' << id_ << ' ' << str <<std::endl;
}

void Philo::SayTaken() const noexcept {
  SaySomething("has taken a fork");
}

void Philo::SayEating() noexcept {
  SaySomething("\x1b[32mis eating\x1b[0m");
  std::this_thread::sleep_for(
      std::chrono::milliseconds (config_->GetTimeToEat()));
  if (config_->IsLimitLunch()) {
    --count_eat_;
  }
}

void Philo::SaySleeping() const noexcept {
  SaySomething("is sleeping");
  std::this_thread::sleep_for(
      std::chrono::milliseconds (config_->GetTimeToSleep()));
}

void Philo::SayThinking() const noexcept {
  SaySomething("is thinking");
}

void Philo::PhiloLive() noexcept {
  if (id_ % 2) {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }
  while(true) {
    SayThinking();
    {
      std::lock_guard<std::mutex> l_fork(*left_fork_);
      SayTaken();
      std::lock_guard<std::mutex> r_fork(*right_fork_);
      SayTaken();
      {
        std::unique_lock lock(mutex_eat_);
        time_last_eat = config_->GetTimer().GetTimeSimulation();
      }
      SayEating();
    }
    if (!count_eat_) {
      return;
    }
    SaySleeping();
    std::this_thread::yield();
  }
}

size_t Philo::GetTimeLastEat() const {
  std::shared_lock lock(mutex_eat_);
  return time_last_eat;
}

void Philo::SayDied() const noexcept {
  config_->GetIoMutex().lock();
  std::cout << "["<< config_->GetTimer().GetTimeSimulation() << "]"
            << ' ' << id_ << ' ' << "died" <<std::endl;
}

int Philo::GetCountEat() const {
  return count_eat_;
}

Philo::Philo(const std::shared_ptr<std::mutex> &right_fork,
             const std::shared_ptr<std::mutex> &left_fork,
             Config *config,
             int id)
    : config_(config),
      id_(id) {
  right_fork_ = right_fork;
  left_fork_ = left_fork;
  count_eat_ = config_->IsLimitLunch() ? config_->GetNumberOfLunch() : 1;
}
