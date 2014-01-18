#pragma once

#include <cmath>
#include <array>
#include <iostream>
#include <memory>
#include <typeinfo>
#include <boost/math/constants/constants.hpp>
#include "configuration.hxx"
#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    class space_converter_t final
    {
    public:
      using float_t = float;
      using a3d_t = std::array<float_t, 3>;
      using a2d_t = std::array<float_t, 2>;
    
    private:
      template<class T> static constexpr typename T::value_type x(const T& a) { return a[0]; }
      template<class T> static constexpr typename T::value_type y(const T& a) { return a[1]; }
      template<class T> static constexpr typename T::value_type z(const T& a) { return a[2]; }
      
      template<class T_in_out, class T_internal = float_t>
      static constexpr T_in_out degrees_to_radians(T_in_out degrees)
      { return T_internal(degrees) * boost::math::constants::pi<T_internal>() / T_internal(180); }
      
      template<class T_in_out, class T_internal = float_t>
      static constexpr T_in_out radians_to_degrees(T_in_out radians)
      { return T_internal(radians) * T_internal(180) / boost::math::constants::pi<T_internal>(); }

      template<class T> static constexpr T pow2(const T v){ return v * v; }
      template<class T> static constexpr T diagonal(const T a, const T b){ return std::sqrt( pow2(a) + pow2(b) ); }

      template<class T> static constexpr T uvalue_to_snorm(const T v, const T max)
      { return (v - (max / 2)) / (max / 2); }
      
      template<class T>
      std::string to_string(const T& vs) const
      {
        std::string r;
        for(auto v: vs)
        {
          r += std::to_string(v);
          r += ",";
        }
        r.resize(r.size() - 1);
        return r;
      }

      const a3d_t top_camera_position_;
      const a3d_t front_camera_position_;
      const float_t top_camera_angle_x_;
      const float_t camera_fov_diagonal_;
      const a2d_t camera_sensor_size_;
      const a2d_t image_size_;
      
      a2d_t camera_fov_div_2_rad_;
      float_t top_camera_angle_x_rad_;
      
    public:
      space_converter_t( const configuration_t& conf);
      
      void initialize();
      
      a3d_t operator()(const a2d_t& top_image_target, const a2d_t& front_image_target) const;
    };
  }
}