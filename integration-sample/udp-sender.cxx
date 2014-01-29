#include "udp-sender.hxx"

namespace arisin
{
  namespace etupirka
  {
    udp_sender_t::udp_sender_t(const configuration_t& conf)
      : address_(conf.udp_sender.address)
      , port_(conf.udp_sender.port)
      , socket(io_service)
    {
      DLOG(INFO) << "address(" << address_ << ") port(" << port_ << "), socket initialized" ;
      
      // try without resolver
      try
      {
        DLOG(INFO) << "try generate endpoint without resolver";
        endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(address_), port_);
      }
      // with resolver
      catch(const std::exception& e)
      {
        DLOG(INFO) << "generate endpoint with resolver";
        boost::asio::ip::udp::resolver        resolver(io_service);
        boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), address_, std::to_string(port_).data());
        endpoint = boost::asio::ip::udp::endpoint(*resolver.resolve(query));
      }
      DLOG(INFO) << "endpoint generated";
      
      socket.open(boost::asio::ip::udp::v4());
      DLOG(INFO) << "socket opened";
    }
    
    void udp_sender_t::operator()(const key_signal_t& key_signal)
    {
      DLOG(INFO) << "key_signal code, state: " << key_signal.code_state.code << "," << key_signal.code_state.state;
      
      using buffer_t = boost::array<decltype(key_signal.char_array)::value_type, sizeof(key_signal.char_array) / sizeof(decltype(key_signal.char_array)::value_type)>;
      const buffer_t& buffer( *reinterpret_cast<const buffer_t*>( key_signal.char_array.data()) );
      
#ifndef NDEBUG
      auto n =
#endif
      socket.send_to(boost::asio::buffer(buffer), endpoint);
#ifndef NDEBUG
      DLOG(INFO) << "message sent [bytes]: " << n;
#endif
    }
    
    void udp_sender_t::operator()(const camera_capture_t::captured_frames_t& captured_frames)
    {
      frame_packet_t frame_packets[2];
      
      frame_packets[0].capture_id = 0;
      frame_packets[1].capture_id = 1;
      frame_packets[0].sequence_id = frame_packets[1].sequence_id = sequence_id;
      
      try
      {
        frame_packets[0].set_data(captured_frames.top);
        frame_packets[1].set_data(captured_frames.front);
      }
      catch(const std::runtime_error& e)
      {
        LOG(WARNING) << "skip frame; frame_packet set exception: " << e.what();
        return;
      }
      
      ++sequence_id;
      
      for(const auto& frame_packet : frame_packets)
      {
        const auto& buffer( frame_packet.mutate_to_const_array() );
      
#ifndef NDEBUG
        auto n =
#endif
        socket.send_to(boost::asio::buffer(buffer), endpoint);
#ifndef NDEBUG
        DLOG(INFO) << "message sent [bytes]: " << n;
#endif
      }
    }
    
    const std::string& udp_sender_t::address() const
    { return address_; }
    
    const int udp_sender_t::port() const
    { return port_; }
  }
}