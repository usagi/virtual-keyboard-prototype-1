#pragma once

#include <string>
#include <unordered_set>
#include <WonderRabbitProject/SQLite3.hpp>
#include "configuration.hxx"
#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    class virtual_keyboard_t final
    {
    public:
      using pressing_keys_t = std::unordered_set<int32_t>;
      
    private:
      WonderRabbitProject::SQLite3::sqlite3_t database_object;
      std::string database_;
      std::string table_;
      WonderRabbitProject::SQLite3::prepare_t statement;
      pressing_keys_t pressing_keys_;
      double x_shift_;
      
    public:
      explicit virtual_keyboard_t(const configuration_t& conf);
      void load_x_shift();
      void reset();
      void add_test(const double x, const double y, const double stroke);
      const pressing_keys_t& pressing_keys() const;
      const std::string& database() const;
      const std::string& table() const;
    };
  }
}