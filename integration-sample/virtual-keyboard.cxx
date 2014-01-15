#include "virtual-keyboard.hxx"

namespace arisin
{
  namespace etupirka
  {
    virtual_keyboard_t::virtual_keyboard_t(const configuration_t& conf)
      : database_object(conf.virtual_keyboard.database)
      , database_(conf.virtual_keyboard.database)
      , table_(conf.virtual_keyboard.table)
    {
      L(INFO, "database(" << database_ << ") table(" << table_ << ")");
      
      load_x_shift();
    }
    
    void virtual_keyboard_t::load_x_shift()
    {
      const auto sql = std::string("select max(x+w) from ") + table();
      L(INFO, "SQL: " << sql);
      x_shift_ = - std::get<0>(database_object.execute_data<double>(sql)[0]) / 2.;
    }
    
    void virtual_keyboard_t::reset()
    { pressing_keys_.clear(); }
    
    void virtual_keyboard_t::add_test(const double x, const double y, const double stroke)
    {
      L(INFO, "x(" << x << ") y(" << y << ") stroke(" << stroke << ")");
      
      const auto x_shifted = x + x_shift_;
      
      auto sql_where = std::string(" where")
        + " x <= " + std::to_string(x_shifted) + " and x + w >= " + std::to_string(x_shifted)
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