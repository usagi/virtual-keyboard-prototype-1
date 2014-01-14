#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    enum class mode_t;
    
    class commandline_helper final
    {
      commandline_helper() = delete;
    public:
      static mode_t interpret(const std::vector<std::string>& arguments);
    };
  }
}
