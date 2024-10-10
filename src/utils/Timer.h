#pragma once

  class Timer {
  public:
    Timer() : start_time_(std::chrono::high_resolution_clock::now()) {};

    ~Timer() {
      Stop();
    }

    double GetTime();

    void Sleep(const double& us_duration);

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;

    void Stop();
  };