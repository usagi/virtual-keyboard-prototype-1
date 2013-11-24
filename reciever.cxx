#include <iostream>
#include <string>
#include <array>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

constexpr auto default_port = 30000;
constexpr auto version_info = "vertual-keyboard-prototype-1/reciever\n"
                              "version 0.0.0";

void recieve(const int port)
{
  using boost::asio::ip::udp;
  
  boost::asio::io_service service;
  udp::socket socket(service, udp::endpoint(udp::v4(), port));
  
  while(true)
  {
    std::array<char, 2048> buffer;
    
    udp::endpoint endpoint;
    boost::system::error_code error;
    auto len = socket.receive_from(boost::asio::buffer(buffer), endpoint, 0, error);
    
    if(error && error != boost::asio::error::message_size)
      throw boost::system::system_error(error);
    
    std::cout.write(buffer.data(), len);
    std::cout << std::endl;
  }
}

boost::program_options::variables_map option(const int& ac, const char* const * const  av)
{
  using namespace boost::program_options;
  
  options_description description("オプション");
  description.add_options()
    ("help,h"   , "ヘルプ")
    ("port,p"   , value<int>()->default_value(default_port)             , "ポート")
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
    recieve(vm["port"].as<int>());
}
catch (const std::exception& e)
{ std::cerr << e.what() << "\n"; }