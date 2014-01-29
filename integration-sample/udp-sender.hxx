#pragma once

#include <string>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "configuration.hxx"
#include "logger.hxx"

#include "camera-capture.hxx"
#include "network-common.hxx"

namespace arisin
{
  namespace etupirka
  {
    class udp_sender_t final
    {
      void initialize();
      
      boost::asio::io_service io_service;
      
      std::string address_;
      int port_;
      
      boost::asio::ip::udp::endpoint        endpoint;
      boost::asio::ip::udp::socket          socket;

      frame_packet_t::sequence_id_t sequence_id;
      
    public:
      udp_sender_t(const configuration_t& conf);
      void operator()(const key_signal_t& key_signal);
      void operator()(const camera_capture_t::captured_frames_t& captured_frames);
      const std::string& address() const;
      const int port() const;
    };
  }
}