#include <iostream>
#include <chrono>
#include <thread>
#include <boost/program_options.hpp>

#include <WonderRabbitProject/key.hxx>

namespace
{
  constexpr auto default_key_name          = "a";
  constexpr auto default_wait_in_sec       = 3;

  constexpr auto version_info = "vertual-keyboard-prototype-1/keyboard-writer\n"
                                "version 0.0.0"
                                ;

  void write_invoke(const std::string key_name, const int wait)
  {
    const auto& kh = WonderRabbitProject::key::key_helper_t::instance();
    const auto& writer = WonderRabbitProject::key::writer_t::instance();
    
    const auto key_code = kh.code(key_name);
    
    for (auto wait_ = wait; wait_ > 0; --wait_)
    {
      std::cout
        << "wait... " << wait_ << " [sec]" << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout
      << "invoke key press: key = "
      << key_name
      << "(" << key_code << (kh.is_valid(key_code) ? "" : " [INVALID]") << ") " << std::endl;
    
    writer(key_code);
  }

  void show_keys()
  {
    const auto& kh = WonderRabbitProject::key::key_helper_t::instance();
    
    const auto data = kh.data_sorted_by_code();
    for(const auto& p : data)
      std::cout
      << "  " << p.first << "(" << p.second << ")"
      << (kh.is_valid(p.first) ? "" : " [INVALID]" )
      << std::endl
      ;
  }

  boost::program_options::variables_map option(const int& ac, const char* const * const  av)
  {
    using namespace boost::program_options;
    
    options_description description("オプション");
    description.add_options()
      ("help,h"   , "ヘルプ")
      ("show-keys,s", "システムで利用可能なキーの名前とコードの一覧を表示")
      ("key,k"    , value<std::string>()->default_value(default_key_name), "キーの名前")
      ("wait,w"   , value<int>()->default_value(default_wait_in_sec)     , "キーイベント発生までのウェイト[秒]")
      ("version,v", "バージョン情報")
      ;
    
    variables_map vm;
    store(parse_command_line(ac, av, description), vm);
    notify(vm);
    
    if(vm.count("help"))
      std::cout << description << std::endl;
    if(vm.count("version"))
      std::cout
        << version_info
        << " - " <<
#if defined(_WIN64) || defined(_WIN32)
          "Windows"
#elif defined(__APPLE__)
          "OSX"
#elif defined(__linux)
          "GNU/Linux"
#else
          "Unknown"
#endif
        << std::endl;
    if(vm.count("show-keys"))
      show_keys();
    return vm;
  }
}

int main (const int ac, const char* const * const av) try
{
  const auto vm = option(ac, av);
  if(!vm.count("help") && !vm.count("version") && !vm.count("show-keys"))
    write_invoke
    ( vm["key"].as<std::string>()
    , vm["wait"].as<int>()
    );
}
catch (const std::exception& e)
{ std::cerr << e.what() << "\n"; }