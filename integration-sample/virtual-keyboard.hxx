#pragma once

#include <string>
#include <unordered_set>
#include <WonderRabbitProject/SQLite3.hpp>
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
      pressing_keys_t pressing_keys_;
      
    public:
      explicit virtual_keyboard_t(const std::string& databse__, const std::string& table__);
      void reset();
      void add_test(const int x, const int y, const int stroke);
      const pressing_keys_t& pressing_keys() const;
      const std::string& database() const;
      const std::string& table() const;
    };
  }
}