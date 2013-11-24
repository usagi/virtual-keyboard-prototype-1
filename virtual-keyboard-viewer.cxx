#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <WonderRabbitProject/SQLite3.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "id_to_name.hxx"

constexpr auto default_database = "virtual-keyboard.sqlite3";
constexpr auto default_table    = "test";
constexpr auto default_scaling  = 3.;

constexpr auto version_info = "vertual-keyboard-prototype-1/virtual-keyboard-viewer\n"
"version 0.0.0";

void view(const std::string& database, const std::string& table, const double scaling)
{
  std::cout
    << "database: " << database << "\n"
       "table   : " << table    << "\n"
    ;
  
  using WonderRabbitProject::SQLite3::sqlite3_t;
  
  sqlite3_t database_object(database);
  
  auto sql = std::string("select * from ") + table;
  auto statement = database_object.prepare(sql);
  auto results   = statement.data<double, double, double, double, double, int32_t>();
  
  cv::Mat image(140 * scaling, 450 * scaling, CV_8UC3, cv::Scalar(204, 204, 255));
  
  for (const auto& row: results)
  {
    const auto& x = std::get<0>(row);
    const auto& y = std::get<1>(row);
    const auto& w = std::get<2>(row);
    const auto& h = std::get<3>(row);
    //const auto& s = std::get<4>(row);
    const auto& id = std::get<5>(row);
    const auto left_top     = cv::Point(x, y) * scaling;
    const auto right_bottom = cv::Point(x + w, y + h) * scaling;
    const auto color        = cv::Scalar(0, 0, 255);
    cv::rectangle(image, left_top, right_bottom, color, 2, CV_AA);
    const auto text_position = cv::Point(x, y + h * .5) * scaling;
    cv::putText(image, id_to_name(id), text_position, cv::FONT_HERSHEY_SIMPLEX, 0.25 * scaling, color, 1, CV_AA);
  }
  
  cv::namedWindow("virtual-keyboard-viewer", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  cv::imshow("virtual-keyboard-viewer", image);
  cv::waitKey(0);
}

boost::program_options::variables_map option(const int& ac, const char* const * const  av)
{
  using namespace boost::program_options;
  
  options_description description("オプション");
  description.add_options()
    ("help,h", "ヘルプ")
    ("database,d", value<std::string>()->default_value(default_database), "データベース")
    ("table,t"   , value<std::string>()->default_value(default_table)   , "テーブル")
    ("scaling,s" , value<double>()->default_value(default_scaling)      , "表示倍率")
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
    view(vm["database"].as<std::string>(), vm["table"].as<std::string>(), vm["scaling"].as<double>());
}
catch (const std::exception& e)
{ std::cerr << e.what() << "\n"; }
