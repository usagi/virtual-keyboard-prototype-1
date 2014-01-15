#include "etupirka.hxx"

namespace arisin
{
  namespace etupirka
  {
    etupirka_t::etupirka_t(const configuration_t& conf)
      : mode_(conf.mode)
    {
      L(INFO, "mode: " << int(conf.mode));
    }
    
    void etupirka_t::run()
    {
      switch(mode_)
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
      L(FATAL, "NOT IMPLEMENTED");
    }
    
    void etupirka_t::run_reciever()
    {
      L(FATAL, "NOT IMPLEMENTED");
    }
    
    mode_t etupirka_t::mode() const
    { return mode_; }
    
    bool etupirka_t::is_running() const
    { return is_running_; }
  }
}