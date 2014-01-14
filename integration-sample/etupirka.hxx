#pragma once

#include <stdexcept>

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
    // 実行モード
    enum class mode_t;
    
    // Etupirka 主制御クラス
    class etupirka_t final
    {
      void initialize(const mode_t);
      void run_main();
      void run_reciever();
      
      mode_t mode_;
      bool   is_running_;
      
      std::shared_ptr<camera_capture_t>   camera_capture;
      std::shared_ptr<finger_detector_t>  finger_detector;
      std::shared_ptr<space_converter_t>  space_converter;
      std::shared_ptr<virtual_keyboard_t> virtual_keyboard;
      std::shared_ptr<udp_sender_t>       udp_sender;
      std::shared_ptr<udp_reciever_t>     udp_reciever;
      std::shared_ptr<key_invoker_t>      key_invoker;
      
    public:
      explicit etupirka_t(const mode_t);
      void run();
      mode_t mode() const;
      bool   is_running() const;
    };
  }
}