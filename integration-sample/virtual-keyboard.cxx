#include "virtual-keyboard.hxx"

namespace arisin
{
  namespace etupirka
  {
    virtual_keyboard_t::virtual_keyboard_t(const configuration_t& conf)
      : database_object(conf.virtual_keyboard.database)
      , database_(conf.virtual_keyboard.database)
      , table_(conf.virtual_keyboard.table)
      , statement(database_object.prepare
        ( "select id"
          " from " + conf.virtual_keyboard.table +
          " where"
          " x <= ? and x + w >= ? and"
          " y <= ? and y + h >= ? and"
          " s <= ?"
        )
      )
    {
      DLOG(INFO) << "database(" << database_ << ") table(" << table_ << ")";
      load_x_shift();
    }
    
    void virtual_keyboard_t::load_x_shift()
    {
      const auto sql = std::string("select max(x+w) from ") + table();
      DLOG(INFO) << "SQL: " << sql;
      
      x_shift_ = - std::get<0>(database_object.execute_data<double>(sql)[0]) / 2.;
      DLOG(INFO) << "x_shift_: " << x_shift_;
    }
    
    void virtual_keyboard_t::reset()
    { pressing_keys_.clear(); }
    
    void virtual_keyboard_t::add_test(const double x, const double y, const double stroke)
    {
      DLOG(INFO) << "x(" << x << ") y(" << y << ") stroke(" << stroke << ")";
      
      const auto x_shifted = x + x_shift_;
      DLOG(INFO) << "x_shifted: " << x_shifted;
      
      statement.reset()
               .bind(x_shifted, 1)
               .bind(x_shifted, 2)
               .bind(y        , 3)
               .bind(y        , 4)
               .bind(stroke   , 5)
               ;
      
      auto results = statement.data<int32_t>();
      
      for (const auto& row: results)
      {
        DLOG(INFO) << "key: id(" << std::get<0>(row) << ")";
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