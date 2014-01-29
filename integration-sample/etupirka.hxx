#pragma once

#include <random>
#include <thread>
#include <chrono>
#include <string>
#ifndef NDEBUG
  #include <sstream>
#endif
#include <stdexcept>
#include <boost/range/algorithm.hpp>
#if __GNUC__ == 4 &&  __GNUC_MINOR__ < 8
  #include <boost/thread/thread.hpp>
#endif
#include "configuration.hxx"
#include "commandline_helper.hxx"
#include "camera-capture.hxx"
#include "finger-detector.hxx"
#include "space-converter.hxx"
#include "virtual-keyboard.hxx"
#include "udp-sender.hxx"
#include "udp-reciever.hxx"
#include "key-invoker.hxx"
#include "gui.hxx"
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
    public:
      static constexpr size_t version_major    = 1;
      static constexpr size_t version_minor    = 0;
      static constexpr size_t version_revision = 0;
      static const std::string version_string()
      { return std::to_string(version_major) + "." + std::to_string(version_minor) + "." + std::to_string(version_revision); }
      
    private:
      void initialize(const mode_t);
      void run_main();
      void run_reciever();
      void run_main_m1();
      void run_reciever_p1();
      void run_dummy_main();
      void run_dummy_reciever();
      
      configuration_t conf_;
      bool is_running_ = false;
      std::chrono::nanoseconds main_loop_wait_;
      
      std::unique_ptr<camera_capture_t>   camera_capture;
      std::unique_ptr<finger_detector_t>  finger_detector_top;
      std::unique_ptr<finger_detector_t>  finger_detector_front;
      std::unique_ptr<space_converter_t>  space_converter;
      std::unique_ptr<virtual_keyboard_t> virtual_keyboard;
      std::unique_ptr<udp_sender_t>       udp_sender;
      std::unique_ptr<udp_reciever_t>     udp_reciever;
      std::unique_ptr<key_invoker_t>      key_invoker;
      
      std::unique_ptr<gui_t>              gui;
      
    public:
      explicit etupirka_t(const configuration_t& conf);
      void initialize();
      void run();
      bool is_running() const;
    };
  }
}