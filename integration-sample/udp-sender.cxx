#include "udp-sender.hxx"

namespace arisin
{
  namespace etupirka
  {
    udp_sender_t::udp_sender_t(const std::string& address__, const int port__)
      : resolver(io_service)
      , query(boost::asio::ip::udp::v4(), address__, std::to_string(port__).data())
      , endpoint(*resolver.resolve(query))
      , socket(io_service)
      , address_(address__)
      , port_(port__)
    {
      L(INFO, "resolver, query, endpoint, socket are initialized");
      L(INFO, "address(" << address__ << ") port(" << port__ << ")" );
      socket.open(boost::asio::ip::udp::v4());
      L(INFO, "socket opened");
    }
    
    void udp_sender_t::operator()(const std::string& message) const
    {
      L(INFO, "message: " << message);
      //socket.send_to(boost::asio::buffer(message), endpoint);
      L(INFO, "message sent");
    }
    
    const std::string& udp_sender_t::address() const
    { return address_; }
    
    const int udp_sender_t::port() const
    { return port_; }
  }
}