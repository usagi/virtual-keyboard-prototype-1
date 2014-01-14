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
      void operator()() const;
    };
  }
}