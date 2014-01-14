#pragma once

#include <WonderRabbitProject/key.hxx>
#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    class key_invoker_t final
    {
    public:
      void operator()(int key_usb_hid_usage_id) const;
    };
  }
}