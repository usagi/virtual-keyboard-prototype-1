#pragma once

#include <array>
#include <string>

namespace arisin
{
  namespace etupirka
  {
      // 実行モード
      enum class mode_t
      { none     // 動作しないモード
      , main     // カメラ制御〜UDP送信モード
      , reciever // UDP受信〜キーストローク発行モード
      };
      
      struct configuration_t
      {
        mode_t mode;
        
        struct camera_capture_configuration_t
        {
          int top_camera_id;
          int front_camera_id;
          int width;
          int height;
        } camera_capture_configuration;
        
        struct finger_detector_configuration_t
        {
          double pre_bilateral_d;
          double pre_bilateral_sc;
          double pre_bilateral_ss;
          
          int pre_morphology_n;
          
          float hsv_h_min, hsv_h_max, hsv_s_min, hsv_s_max, hsv_v_min, hsv_v_max;
          
          int nail_morphology_n;
          int nail_median_blur_ksize;
          
          double circles_dp;
          double circles_min_dist;
          double circles_param_1;
          double circles_param_2;
          int circles_min_radius;
          int circles_max_radius;
        } finger_detector_configuration;
        
        struct space_converter_configuration_t
        {
          using float_t = float;
          using a3d_t = std::array<float_t, 3>;
          using a2d_t = std::array<float_t, 2>;
          
          a3d_t top_camera_position;
          a3d_t front_camera_position;
          float_t top_camera_angle_x;
          float_t camera_fov_diagonal;
          a2d_t camera_sensor_size;
          a2d_t image_size;
        } space_converter_configuration;
        
        struct virtual_keyboard_configuration_t
        {
          std::string database;
          std::string table;
        } virtual_keyboard_configuration;
        
        struct udp_sender_configuration_t
        {
          std::string address;
          int port;
        } udp_sender_configuration;
        
        struct udp_reciver_configuration_t
        {
          int port;
        } udp_reciver_configuration;
        
        struct key_invoker_configuration_t
        {
          
        } key_invoker_configuration;
      };
  }
}
