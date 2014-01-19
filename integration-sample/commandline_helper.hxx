#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cmath>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm.hpp>

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
      static void show_conf(const configuration_t& conf, std::ostream& out = std::cout);
      static void save_conf(const configuration_t& conf, const std::string& filename = "etupirka.conf");
      static void show_default();
      static configuration_t load_default();
      static void load_file(configuration_t& conf, const std::string& filename = "etupirka.conf");
      static boost::property_tree::ptree boost_ptree(const configuration_t& conf);
    };
  }
}
