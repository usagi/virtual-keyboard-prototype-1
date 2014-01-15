#pragma once

#include <string>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include "configuration.hxx"
#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    class udp_sender_t final
    {
      void initialize();
      
      boost::asio::io_service io_service;
      
      boost::asio::ip::udp::resolver        resolver;
      boost::asio::ip::udp::resolver::query query;
      boost::asio::ip::udp::endpoint        endpoint;
      boost::asio::ip::udp::socket          socket;
      
      std::string address_;
      int port_;
      
    public:
      udp_sender_t(const configuration_t& conf);
      void operator()(const key_signal_t& key_signal);
      const std::string& address() const;
      const int port() const;
    };
  }
}