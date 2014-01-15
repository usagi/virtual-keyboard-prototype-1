#pragma once

#include <stdexcept>
#include <boost/range/algorithm.hpp>

#include "configuration.hxx"
#include "commandline_helper.hxx"
#include "camera-capture.hxx"
#include "finger-detector.hxx"
#include "space-converter.hxx"
#include "virtual-keyboard.hxx"
#include "udp-sender.hxx"
#include "udp-reciever.hxx"
#include "key-invoker.hxx"
#include "logger.hxx"

// created by arisin: https://github.com/arisin
namespace arisin
{
  // the Etupirka project
  namespace etupirka
  {
    // Etupirka 主制御クラス
    class etupirka_t final
    {
      void initialize(const mode_t);
      void run_main();
      void run_reciever();
      
      configuration_t conf_;
      bool is_running_;
      
      std::unique_ptr<camera_capture_t>   camera_capture;
      std::unique_ptr<finger_detector_t>  finger_detector_top;
      std::unique_ptr<finger_detector_t>  finger_detector_front;
      std::unique_ptr<space_converter_t>  space_converter;
      std::unique_ptr<virtual_keyboard_t> virtual_keyboard;
      std::unique_ptr<udp_sender_t>       udp_sender;
      std::unique_ptr<udp_reciever_t>     udp_reciever;
      std::unique_ptr<key_invoker_t>      key_invoker;
      
    public:
      explicit etupirka_t(const configuration_t& conf);
      void initialize();
      void run();
      bool is_running() const;
    };
  }
}