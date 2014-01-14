#pragma once

#include <string>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    class udp_reciever_t final
    {
      boost::asio::io_service      io_service;
      boost::asio::ip::udp::socket socket;
      int port_;
    public:
      udp_reciever_t(const int port__);
      std::string operator()();
      const int port() const;
    };
  }
}