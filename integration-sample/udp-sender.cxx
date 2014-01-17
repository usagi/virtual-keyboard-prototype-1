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
      DLOG(INFO) << "resolver, query, endpoint, socket are initialized";
      DLOG(INFO) << "address(" << address_ << ") port(" << port_ << ")" ;
      socket.open(boost::asio::ip::udp::v4());
      DLOG(INFO) << "socket opened";
    }
    
    void udp_sender_t::operator()(const key_signal_t& key_signal)
    {
      DLOG(INFO) << "key_signal code, state: " << key_signal.code_state.code << "," << key_signal.code_state.state;
#ifndef NDEBUG
      auto n =
#endif
      socket.send_to(boost::asio::buffer(key_signal.char_array), endpoint);
      DLOG(INFO) << "message sent [bytes]: " << n;
    }
    
    const std::string& udp_sender_t::address() const
    { return address_; }
    
    const int udp_sender_t::port() const
    { return port_; }
  }
}