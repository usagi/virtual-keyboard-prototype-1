#pragma once

#include <chrono>
#include <string>
#include <iostream>

namespace
{

  inline const std::chrono::microseconds time(const std::function<void()>& f)
  {
    const auto begin = std::chrono::high_resolution_clock::now();
    f();
    return std::chrono::duration_cast<std::chrono::microseconds>
      (std::chrono::high_resolution_clock::now() - begin);
  }

  inline void print_time(const std::function<void()>& f, const std::string& title = "unknown")
  {
    auto t = time(f);
    std::cout << "time of " << title << ": " << t.count() << " [us]\n";
  }

}
