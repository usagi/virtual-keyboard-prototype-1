#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <WonderRabbitProject/SQLite3.hpp>

constexpr auto default_database = "virtual-keyboard.sqlite3";
constexpr auto default_table    = "test";
constexpr auto default_x = 150;
constexpr auto default_y =  50;
constexpr auto default_s =  5;

constexpr auto version_info = "vertual-keyboard-prototype-1/virtual-keyboard-tester\n"
"version 0.0.0";

const std::string id_to_name(const int id)
{
  switch(id)
  {
    case 110: return "Esc";
    case 112: return "F1";
    case 113: return "F2";
    case 114: return "F3";
    case 115: return "F4";
    case 116: return "F5";
    case 117: return "F6";
    case 118: return "F7";
    case 119: return "F8";
    case 120: return "F9";
    case 121: return "F10";
    case 122: return "F11";
    case 123: return "F12";
    case 124: return "Print Scr.";
    case 125: return "Scroll Lock";
    case 126: return "Pause";
    case 1: return "半角／全角";
    case 2: return "1";
    case 3: return "2";
    case 4: return "3";
    case 5: return "4";
    case 6: return "5";
    case 7: return "6";
    case 8: return "7";
    case 9: return "8";
    case 10: return "9";
    case 11: return "0";
    case 12: return "-";
    case 13: return "^";
    case 14: return "\\(yen)";
    case 15: return "Back space";
    case 16: return "Tab";
    case 17: return "Q";
    case 18: return "W";
    case 19: return "E";
    case 20: return "R";
    case 21: return "T";
    case 22: return "Y";
    case 23: return "U";
    case 24: return "I";
    case 25: return "O";
    case 26: return "P";
    case 27: return "@";
    case 28: return "[";
    case 30: return "CapsLk";
    case 31: return "A";
    case 32: return "S";
    case 33: return "D";
    case 34: return "F";
    case 35: return "G";
    case 36: return "H";
    case 37: return "J";
    case 38: return "K";
    case 39: return "L";
    case 40: return ";";
    case 41: return ":";
    case 42: return "]";
    case 43: return "Enter";
    case 44: return "Shift(left)";
    case 46: return "Z";
    case 47: return "X";
    case 48: return "C";
    case 49: return "V";
    case 50: return "B";
    case 51: return "N";
    case 52: return "M";
    case 53: return ",";
    case 54: return ".";
    case 55: return "/";
    case 56: return "\\(back slash)";
    case 57: return "Shift(right)";
    case 58: return "Ctrl(left)";
    case 127: return "Win.(left)";
    case 60: return "Alt(left)";
    case 131: return "無変換";
    case 61: return "Space";
    case 132: return "変換";
    case 133: return "カタカナ ひらがな";
    case 62: return "Alt(right)";
    case 128: return "Win.(right)";
    case 129: return "App.";
    case 64: return "Ctrl(right)";
    case 75: return "Insert";
    case 76: return "Delete";
    case 80: return "Home";
    case 81: return "End";
    case 85: return "Page Up";
    case 86: return "Page Down";
    case 83: return "Up";
    case 79: return "Left";
    case 84: return "Down";
    case 89: return "Right";
    case 90: return "Num Lock";
    case 91: return "Num 7";
    case 92: return "Num 4";
    case 93: return "Num 1";
    case 95: return "Num /";
    case 96: return "Num 8";
    case 97: return "Num 5";
    case 98: return "Num 2";
    case 99: return "Num 0";
    case 100: return "Num *";
    case 101: return "Num 9";
    case 102: return "Num 6";
    case 103: return "Num 3";
    case 104: return "Num .";
    case 105: return "Num -";
    case 106: return "Num +";
    case 108: return "Num Enter";
  };
  
  return "????";
}

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