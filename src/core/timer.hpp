// timer.hpp
#pragma once
#include <chrono>
namespace g101 {
class Timer {
public:
    void start() { t0_ = clock::now(); }
    double ms()  const { return std::chrono::duration<double, std::milli>(clock::now() - t0_).count(); }
    double us()  const { return std::chrono::duration<double, std::micro>(clock::now() - t0_).count(); }
private:
    using clock = std::chrono::steady_clock;
    clock::time_point t0_;
};
}