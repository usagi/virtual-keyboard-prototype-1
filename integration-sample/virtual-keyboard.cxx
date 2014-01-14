#include "virtual-keyboard.hxx"

namespace arisin
{
  namespace etupirka
  {
    virtual_keyboard_t::virtual_keyboard_t(const std::string& database__, const std::string& table__)
      : database_object(database__)
      , database_(database__)
      , table_(table__)
    {
      L(INFO, "ctor: database(" << database__ << ") table(" << table__ << ")");
    }
    
    void virtual_keyboard_t::reset()
    { pressing_keys_.clear(); }
    
    void virtual_keyboard_t::add_test(const int x, const int y, const int stroke)
    {
      L(INFO, "x(" << x << ") y(" << y << ") stroke(" << stroke << ")");

      auto sql_where = std::string(" where")
        + " x <= " + std::to_string(x) + " and x + w >= " + std::to_string(x)
        + " and"
        + " y <= " + std::to_string(y) + " and y + h >= " + std::to_string(y)
        + " and"
        + " s <= " + std::to_string(stroke)
        ;
      auto sql = std::string("select id from ") + table() + sql_where;
      L(INFO, "SQL: " << sql);
      
      auto statement = database_object.prepare(sql);
      auto results = statement.data<int32_t>();
      
      for (const auto& row: results)
      {
        L(INFO, "key: id(" << std::get<0>(row) << ")");
        pressing_keys_.emplace(std::get<0>(row));
      }
    }
    
    const virtual_keyboard_t::pressing_keys_t& virtual_keyboard_t::pressing_keys() const
    { return pressing_keys_; }
    
    const std::string& virtual_keyboard_t::database() const
    { return database_; }
    
    const std::string& virtual_keyboard_t::table() const
    { return table_; }
  }
}