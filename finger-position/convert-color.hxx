#pragma once

#include <cstdint>

namespace WonderRabbitProject
{
  namespace color
  {
    template<class T>
    struct rgb_t { T r, g, b; };
    
    using rgb_uint8_t  = rgb_t<uint8_t>;
    using rgb_uint16_t = rgb_t<uint16_t>;
    using rgb_uint32_t = rgb_t<uint32_t>;
    using rgb_uint64_t = rgb_t<uint64_t>;
    
    using rgb_int8_t  = rgb_t<int8_t>;
    using rgb_int16_t = rgb_t<int16_t>;
    using rgb_int32_t = rgb_t<int32_t>;
    using rgb_int64_t = rgb_t<int64_t>;
    
    using rgb_float32_t = rgb_t<float>;
    using rgb_float64_t = rgb_t<double>;
    
    template<class T>
    struct hsl_t { T h, s, l; };
    
    using hsl_uint8_t  = hsl_t<uint8_t>;
    using hsl_uint16_t = hsl_t<uint16_t>;
    using hsl_uint32_t = hsl_t<uint32_t>;
    using hsl_uint64_t = hsl_t<uint64_t>;
    
    using hsl_int8_t  = hsl_t<int8_t>;
    using hsl_int16_t = hsl_t<int16_t>;
    using hsl_int32_t = hsl_t<int32_t>;
    using hsl_int64_t = hsl_t<int64_t>;
    
    using hsl_float32_t = hsl_t<float>;
    using hsl_float64_t = hsl_t<double>;
    
    template<class T>
    const bool operator<(const rgb_t<T>&a, const rgb_t<T>& b)
    {
      if (a.b > b.b || a.g > b.g)
        return false;
      return a.r < b.r;
    }
    
    
  }
}
