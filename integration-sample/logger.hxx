#pragma once

#ifndef NDEBUG
  #include <glog/logging.h>
  #define L(a,b) LOG(a) << b
  #define ARISIN_ETUPIRIKA_LOGGER 1
#else
  #define L(a,b)
#endif

namespace arisin
{
  namespace etupirka
  {
    namespace logger
    {
      void initialize(const bool output_to_stderr = true);
    }
  }
}