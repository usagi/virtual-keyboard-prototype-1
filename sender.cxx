#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

constexpr auto default_address = "localhost";
constexpr auto default_port    = 30000;
constexpr auto default_message = "hello from sender";

constexpr auto version_info = "vertual-keyboard-prototype-1/sender\n"
                              "version 0.0.0";

void send(const std::string& address, const int port, const std::string message)
{
  using boost::asio::ip::udp;
  
  boost::asio::io_service service;
  
  udp::resolver resolver(service);
  decltype(resolver)::query query(udp::v4(), address, std::to_string(port).data());
  udp::endpoint endpoint = *resolver.resolve(query);
  
  udp::socket socket(service);
  socket.open(udp::v4());
  
  socket.send_to(boost::asio::buffer(message), endpoint);
}

boost::program_options::variables_map option(const int& ac, const char* const * const  av)
{
  using namespace boost::program_options;
  
  options_description description("オプション");
  description.add_options()
    ("help,h"   , "ヘルプ")
    ("address,a", value<std::string>()->default_value(default_address)  , "アドレス")
    ("port,p"   , value<int>()->default_value(default_port)             , "ポート")
    ("message,m", value<std::string>()->default_value(default_message)  , "メッセージ")
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
    send(vm["address"].as<std::string>(), vm["port"].as<int>(), vm["message"].as<std::string>());
}
catch (const std::exception& e)
{ std::cerr << e.what() << "\n"; }