#include "udp-reciever.hxx"

namespace arisin
{
  namespace etupirka
  {
    udp_reciever_t::udp_reciever_t(const int port__)
      : socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port__))
      , port_(port__)
    {
      L(INFO, "socket is initialized");
      L(INFO, "port(" << port__ << ")" );
    }
    
    std::string udp_reciever_t::operator()()
    {
      using boost::asio::ip::udp;
      
      boost::array<char, 4096>  buffer;
      udp::endpoint             endpoint;
      boost::system::error_code error;
      
      auto len = socket.receive_from(boost::asio::buffer(buffer), endpoint, 0, error);
      
      L(INFO, "result of socket.recieve_from: len(" << len << ") endpoint(" << endpoint.address().to_string() << ") error(" << error << ")");
      
      if(error && error != boost::asio::error::message_size)
        throw boost::system::system_error(error);
      
      L(INFO, "recieve message" << std::string(buffer.data(), buffer.data()));
      
      return { reinterpret_cast<const char*>(buffer.data()), size_t(len) };
    }
    
    const int udp_reciever_t::port() const
    { return port_; }
  }
}