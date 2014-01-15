#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

#include "configuration.hxx"
#include "logger.hxx"
#include "etupirka.hxx"

namespace arisin
{
  namespace etupirka
  {
    class commandline_helper_t final
    {
    public:
      static configuration_t interpret(const std::vector<std::string>& arguments);
      static void show_help();
    };
  }
}
