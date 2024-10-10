#include <chrono>
#include <thread>
#include <stdint.h>
#include <iostream>

#include "Timer.h"

  double Timer::GetTime() {
      auto end_time = std::chrono::high_resolution_clock::now();

      auto start = std::chrono::time_point_cast<std::chrono::microseconds>(start_time_).time_since_epoch().count();
      auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time).time_since_epoch().count();

      return end - start;
  }
  void Timer::Sleep(const double& us_duration) {
    std::this_thread::sleep_for(std::chrono::microseconds(uint32_t(us_duration)));
  }

  void Timer::Stop() {
    auto us = GetTime();
    double ms = us * 0.001;

    std::cout << ms << std::endl;
}