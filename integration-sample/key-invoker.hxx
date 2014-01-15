#pragma once

#include <unordered_set>
#include <WonderRabbitProject/key.hxx>
#include "configuration.hxx"
#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    class key_invoker_t final
    {
    public:
      using pressing_keys_t = std::unordered_set<int32_t>;
      
    private:
      pressing_keys_t pressing_keys;
      
    public:
      key_invoker_t(const configuration_t& conf);
      ~key_invoker_t();
      void operator()(int key_usb_hid_usage_id, WonderRabbitProject::key::writer_t::state_t key_state);
    };
  }
}