#pragma once

#include <string>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "configuration.hxx"
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
      udp_reciever_t(const configuration_t& conf);
      key_signal_t operator()();
      const int port() const;
    };
  }
}