#pragma once

#include <stdexcept>

#include "logger.hxx"
#include "commandline_helper.hxx"
#include "camera-capture.hxx"
#include "finger-detector.hxx"
#include "space-converter.hxx"
#include "virtual-keyboard.hxx"
#include "udp-sender.hxx"
#include "udp-reciever.hxx"
#include "key-invoker.hxx"

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
      camera_capture_t 
    public:
      explicit etupirka_t(const mode_t);
      void run();
    };
  }
}