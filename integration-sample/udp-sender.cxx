#include "udp-sender.hxx"

namespace arisin
{
  namespace etupirka
  {
    udp_sender_t::udp_sender_t(const configuration_t& conf)
      : resolver(io_service)
      , query(boost::asio::ip::udp::v4(), conf.udp_sender.address, std::to_string(conf.udp_sender.port).data())
      , endpoint(*resolver.resolve(query))
      , socket(io_service)
      , address_(conf.udp_sender.address)
      , port_(conf.udp_sender.port)
    {
      L(INFO, "resolver, query, endpoint, socket are initialized");
      L(INFO, "address(" << address_ << ") port(" << port_ << ")" );
      socket.open(boost::asio::ip::udp::v4());
      L(INFO, "socket opened");
    }
    
    void udp_sender_t::operator()(const std::string& message) const
    {
      L(INFO, "message: " << message);
#ifndef NDEBUG
      //auto n =
#endif
      // ToDo: ????
      //socket.send_to(boost::asio::buffer(message), endpoint);
      //L(INFO, "message sent [bytes]: " << n);
    }
    
    const std::string& udp_sender_t::address() const
    { return address_; }
    
    const int udp_sender_t::port() const
    { return port_; }
  }
}