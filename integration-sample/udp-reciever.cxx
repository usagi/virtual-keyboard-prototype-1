#include "udp-reciever.hxx"

namespace arisin
{
  namespace etupirka
  {
    udp_reciever_t::udp_reciever_t(const configuration_t& conf)
      : socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), conf.udp_sender.port))
      , port_(conf.udp_reciever.port)
    {
      DLOG(INFO) << "socket is initialized";
      DLOG(INFO) << "port(" << port_ << ")" ;
    }
    
    key_signal_t udp_reciever_t::operator()()
    {
      using boost::asio::ip::udp;
      
      //boost::array<char, 8> buffer;
      key_signal_t          key_signal;
      udp::endpoint         endpoint;
      boost::system::error_code error;
      
      DLOG(INFO) << "begin wait for socket_recieve_from";
      
      //auto len = socket.receive_from(boost::asio::buffer(buffer), endpoint, 0, error);
      auto len = socket.receive_from(boost::asio::buffer(key_signal.char_array), endpoint, 0, error);
      
      DLOG(INFO) << "result of socket.recieve_from: len(" << len << ") endpoint(" << endpoint.address().to_string() << ") error(" << error << ")";
      
      if(error && error != boost::asio::error::message_size)
        throw boost::system::system_error(error);
      
      //DLOG(INFO) << "recieve message" << std::string(buffer.data(), buffer.data());
      DLOG(INFO) << "recieve key_signal code state: " << key_signal.code_state.code << ", " << key_signal.code_state.state;
      
      //return { reinterpret_cast<const char*>(buffer.data()), size_t(len) };
      return std::move(key_signal);
    }
    
    const int udp_reciever_t::port() const
    { return port_; }
  }
}