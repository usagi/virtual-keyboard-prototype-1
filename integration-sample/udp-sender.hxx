#pragma once

#include <string>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

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
      udp_sender_t(const std::string& address__ = "localhost", const int port = 30000);
      void operator()(const std::string& message) const;
      const std::string& address() const;
      const int port() const;
    };
  }
}