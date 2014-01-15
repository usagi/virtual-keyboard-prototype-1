#include "etupirka.hxx"

namespace arisin
{
  namespace etupirka
  {
    etupirka_t::etupirka_t(const configuration_t& conf)
      : conf_(conf)
    {
      L(INFO, "conf.mode: " << int(conf.mode));
    }
    
    void etupirka_t::run()
    {
      if(is_running_)
      {
        L(WARNING, "etupirka(" << this << ") is already running, return");
        return;
      }
        
      switch(conf_.mode)
      {
        case mode_t::main:
          L(INFO, "mode is main, to run_main");
          run_main();
          return;
          
        case mode_t::reciever:
          L(INFO, "mode is reciever, to run_reciever");
          run_reciever();
          return;
          
        case mode_t::none:
        default:
          L(INFO, "mode is none, return");
          return;
      }
    }
    
    void etupirka_t::run_main()
    {
      initialize();
      
      L(FATAL, "NOT IMPLEMENTED");
    }
    
    void etupirka_t::run_reciever()
    {
      initialize();
      
      is_running_ = true;
      
      L(INFO, "run reciever mode main loop");
      
      while(is_running_)
      {
        const auto key_signal = (*udp_reciever)();
        (*key_invoker)(key_signal.code_state.code, WonderRabbitProject::key::writer_t::state_t(key_signal.code_state.state));
      }
      
      L(INFO, "exit main loop");
    }
    
    void etupirka_t::initialize()
    {
      L(INFO, "initialize");
      switch(conf_.mode)
      {
        case mode_t::main:
          camera_capture.reset(new camera_capture_t(conf_));
          finger_detector_top.reset(new finger_detector_t(conf_, true));
          finger_detector_front.reset(new finger_detector_t(conf_, false));
          space_converter.reset(new space_converter_t(conf_));
          virtual_keyboard.reset(new virtual_keyboard_t(conf_));
          udp_sender.reset(new udp_sender_t(conf_));
          udp_reciever.reset(nullptr);
          key_invoker.reset(nullptr);
          break;
          
        case mode_t::reciever:
          camera_capture.reset(nullptr);
          finger_detector_top.reset(nullptr);
          finger_detector_front.reset(nullptr);
          space_converter.reset(nullptr);
          virtual_keyboard.reset(nullptr);
          udp_sender.reset(nullptr);
          udp_reciever.reset(new udp_reciever_t(conf_));
          key_invoker.reset(new key_invoker_t(conf_));
          break;
          
        default:
          camera_capture.reset(nullptr);
          finger_detector_top.reset(nullptr);
          finger_detector_front.reset(nullptr);
          space_converter.reset(nullptr);
          virtual_keyboard.reset(nullptr);
          udp_sender.reset(nullptr);
          udp_reciever.reset(nullptr);
          key_invoker.reset(nullptr);
      }
      
      L(INFO, camera_capture.get());
      L(INFO, finger_detector_top.get());
      L(INFO, finger_detector_front.get());
      L(INFO, space_converter.get());
      L(INFO, virtual_keyboard.get());
      L(INFO, udp_sender.get());
      L(INFO, udp_reciever.get());
      L(INFO, key_invoker.get());
      
    }
    
    bool etupirka_t::is_running() const
    { return is_running_; }
  }
}