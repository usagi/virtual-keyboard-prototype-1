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
      
      key_signal_t           key_signal;
      
      using buffer_t = boost::array<decltype(key_signal.char_array)::value_type, sizeof(key_signal.char_array) / sizeof(decltype(key_signal.char_array)::value_type)>;
      buffer_t& buffer( *reinterpret_cast<buffer_t*>( key_signal.char_array.data()) );
      
      udp::endpoint          endpoint;
      boost::system::error_code error;
      
      DLOG(INFO) << "begin wait for socket_recieve_from";
      
      auto len = socket.receive_from(boost::asio::buffer(buffer), endpoint, 0, error);
      
      DLOG(INFO) << "result of socket.recieve_from: len(" << len << ") endpoint(" << endpoint.address().to_string() << ") error(" << error << ")";
      
      if(error && error != boost::asio::error::message_size)
        throw boost::system::system_error(error);
      
      DLOG(INFO) << "recieve key_signal code state: " << key_signal.code_state.code << ", " << key_signal.code_state.state;
      
      return key_signal;
    }
    
    camera_capture_t::captured_frames_t udp_reciever_t::recieve_captured_frames()
    {
      LOG(FATAL) << "NOT IMPLEMENTED";
      
      using boost::asio::ip::udp;
      
      camera_capture_t::captured_frames_t captured_frames;
      frame_packet_t frame_packets[2];
      
      do
      {
        frame_packet_t frame_packet;
        
        auto& buffer( frame_packet.mutate_to_array() );
        
        udp::endpoint          endpoint;
        boost::system::error_code error;
        
        DLOG(INFO) << "begin wait for socket_recieve_from";
        
        auto len = socket.receive_from(boost::asio::buffer(buffer), endpoint, 0, error);
        
        DLOG(INFO) << "result of socket.recieve_from: len(" << len << ") endpoint(" << endpoint.address().to_string() << ") error(" << error << ")";
        
        if(error && error != boost::asio::error::message_size)
          throw boost::system::system_error(error);
        
        frame_packets[frame_packet.capture_id] = std::move(frame_packet);
      }
      while(frame_packets[0].real_data_size > 0 && frame_packets[0].sequence_id == frame_packets[1].sequence_id);
      
      captured_frames.top   = cv::imdecode(cv::Mat(frame_packets[0].to_vector(), true), CV_LOAD_IMAGE_COLOR);
      captured_frames.front = cv::imdecode(cv::Mat(frame_packets[1].to_vector(), true), CV_LOAD_IMAGE_COLOR);
      
      return captured_frames;
    }
    
    const int udp_reciever_t::port() const
    { return port_; }
  }
}