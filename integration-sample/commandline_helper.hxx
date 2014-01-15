#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include "configuration.hxx"
#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    class commandline_helper final
    {
    public:
      static configuration_t interpret(const std::vector<std::string>& arguments);
    };
  }
}
