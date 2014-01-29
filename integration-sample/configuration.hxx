#pragma once

#include <array>
#include <string>
#include <stdexcept>

#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    // 実行モード
    enum class mode_t
    { none           // 動作しないモード
    , main           // ｛カメラ制御→画像処理→キーシグナル生成→UDP送信（キーシグナル）｝モード
    , reciever       // ｛UDP受信（キーシグナル）→キーストローク発行｝モード
    , main_m1        // ｛カメラ制御→UDP送信（画像）｝モード
    , reciever_p1    // ｛UDP受信（画像）→画像処理→キーシグナル生成→キーストローク発行｝モード
    , dummy_main     // ｛ランダムにキーシグナルを生成→UDP送信（キーシグナル）｝モード
    , dummy_reciever // ｛ランダムにキーシグナルを生成→キーストローク発行｝モード
    };
    
    struct configuration_t
    {
      mode_t mode;
      
      bool gui;
      
      int fps;
      
      std::string video_file_top;
      std::string video_file_front;
      
      struct camera_capture_configuration_t
      {
        int top_camera_id;
        int front_camera_id;
        int width;
        int height;
      } camera_capture;
      
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
      } finger_detector_top
      , finger_detector_front;
      
      float circle_x_distance_threshold;
      
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
      } space_converter;
      
      struct virtual_keyboard_configuration_t
      {
        std::string database;
        std::string table;
      } virtual_keyboard;
      
      struct udp_sender_configuration_t
      {
        std::string address;
        int port;
      } udp_sender;
      
      struct udp_reciever_configuration_t
      {
        int port;
      } udp_reciever;
      
      bool send_repeat_key_down_signal;
      bool recieve_repeat_key_down_signal;
      
      struct key_invoker_configuration_t
      {
        
      } key_invoker;
    };
    
    union key_signal_t
    {
      struct code_state_t
      {
        uint32_t code;
        uint8_t  state;
      } code_state;
      
      std::array<char, sizeof(code_state_t)> char_array;
      
      explicit key_signal_t(decltype(code_state_t::code) code_ = -1, decltype(code_state_t::state) state_ = -1)
        : code_state({code_, uint8_t(state_)})
      { }
    };
    
  }
}
