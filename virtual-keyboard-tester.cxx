#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <WonderRabbitProject/SQLite3.hpp>

#include "id_to_name.hxx"

constexpr auto default_database = "virtual-keyboard.sqlite3";
constexpr auto default_table    = "test";
constexpr auto default_x = 150;
constexpr auto default_y =  50;
constexpr auto default_s =  5;

constexpr auto version_info = "vertual-keyboard-prototype-1/virtual-keyboard-tester\n"
"version 0.0.0";

void test(const std::string& database, const std::string& table, const int x, const int y, const int s)
{
  std::cerr
    << "database: " << database << "\n"
       "table   : " << table    << "\n"
       "x       : " << x << "\n"
       "y       : " << y << "\n"
       "s       : " << s << "\n"
    ;

  using WonderRabbitProject::SQLite3::sqlite3_t;

  sqlite3_t database_object(database);
  
  auto sql_where = std::string(" where")
    + " x <= " + std::to_string(x) + " and x + w >= " + std::to_string(x)
    + " and"
    + " y <= " + std::to_string(y) + " and y + h >= " + std::to_string(y)
    + " and"
    + " s <= " + std::to_string(s)
    ;
  auto sql = std::string("select * from ") + table + sql_where;
  auto statement = database_object.prepare(sql);
  auto results   = statement.data<double, double, double, double, double, int32_t>();
  
  for (const auto& row: results)
    std::cerr << 
      "x: " << std::get<0>(row) << " | "
      "y: " << std::get<1>(row) << " | "
      "w: " << std::get<2>(row) << " | "
      "h: " << std::get<3>(row) << " | "
      "s: " << std::get<4>(row) << " | "
      "id: "<< std::get<5>(row) << " "
      "["   << id_to_name(std::get<5>(row)) << "]"
      "\n"
      ;
  
}

boost::program_options::variables_map option(const int& ac, const char* const * const  av)
{
  using namespace boost::program_options;
  
  options_description description("オプション");
  description.add_options()
  ("help,h", "ヘルプ")
  ("database,d", value<std::string>()->default_value(default_database), "データベース")
  ("table,t"   , value<std::string>()->default_value(default_table)   , "テーブル")
  ("x,x", value<int>()->default_value(default_x), "入力X座標")
  ("y,y", value<int>()->default_value(default_y), "入力Y座標")
  ("s,s", value<int>()->default_value(default_s), "入力S座標（ストローク深さ）")
  ("version,v", "バージョン情報")
  ;
  
  variables_map vm;
  store(parse_command_line(ac, av, description), vm);
  notify(vm);
  
  if(vm.count("help"))
    std::cout << description << std::endl;
  if(vm.count("version"))
    std::cout << version_info << std::endl;
  
  return vm;
}

int main (const int ac, const char* const * const av) try
{
  auto vm = option(ac, av);
  if(!vm.count("help") && !vm.count("version"))
    test
      ( vm["database"].as<std::string>(), vm["table"].as<std::string>()
      , vm["x"].as<int>(), vm["y"].as<int>(), vm["s"].as<int>()
      );
}
catch (const std::exception& e)
{ std::cerr << e.what() << "\n"; }