#include "commandline_helper.hxx"

namespace
{
  constexpr size_t h(const char* const  str, int n = 0)
  { return !str[n] ? 5381 : (h(str, n+1)*33) ^ str[n]; }
  
  std::string to_string(const arisin::etupirka::mode_t m)
  {
    switch(m)
    {
      case arisin::etupirka::mode_t::none: return "none";
      case arisin::etupirka::mode_t::main: return "main";
      case arisin::etupirka::mode_t::reciever: return "reciever";
    }
    L(FATAL, "unkown mode: " << int(m));
    throw std::runtime_error(std::string("unkown mode: ") + std::to_string(int(m)));
  }
  
        
  template<class T>
  std::string to_string(const T& vs)
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
  
}

namespace arisin
{
  namespace etupirka
  {
    configuration_t commandline_helper_t::interpret(const std::vector<std::string>& arguments)
    {
      L(INFO, "interpert");
#ifndef NDEBUG
      for(size_t n = 0; n < arguments.size(); ++n)
        L(INFO, "arguments[" << n << "]: " << arguments[n]);
#endif
      
      // もし、コマンドラインに -dまたは--default-confがあればshow_defaultして終わる
      if(std::find_if(std::begin(arguments), std::end(arguments), [](const std::string& a){ return a == "-d" || a == "--default-conf"; }) != std::end(arguments))
      {
        L(INFO, "found -d(--default-conf) option in commandline arguments");
        show_default();
        exit(0);
      }
      
      // デフォルト値のロード
      auto conf = commandline_helper_t::load_default();
      
      // ファイルからのロード
      {
        const auto i = std::find_if(std::begin(arguments), std::end(arguments), [](const std::string& a){ return a == "-c" || a == "--conf-file"; } );
        load_file(conf, i == std::end(arguments) ? "etupirka.conf" : *i);
      }
      
      // もし、コマンドラインに -mまたは--modeが不在かつconf.mode=mode_t::noneの場合は強制的にここでヘルプを出して終わる。
      if( conf.mode == mode_t::none
       && std::none_of(std::begin(arguments), std::end(arguments), [](const std::string& a){ return a == "-m" || a == "--mode"; })
      )
      {
        L(WARNING, "conf.mode=none and arguments not include -m(--mode) option. force show help and exit");
        show_help();
        exit(0);
      }
      
      // コマンドラインパーサー
      for(auto i = std::begin(arguments), e = std::end(arguments); i < e; ++i)
      {
        
        switch(h(i->data()))
        {
          case h("-m"):
          case h("--mode"):
            try
            {
              switch(h((++i)->data()))
              {
                case h("main"    ): conf.mode = mode_t::main;     break;
                case h("reciever"): conf.mode = mode_t::reciever; break;
                default: conf.mode = mode_t::none;
              }
            }
            catch(const std::exception& e) { L(ERROR, "catch exception: " << e.what()); }
            continue;
            
          case h("-t"):
          case h("--camera-capture/top-camera-id"):
            try { conf.camera_capture.top_camera_id = std::stoi(*++i); }
            catch(const std::exception& e) { L(ERROR, "catch exception: " << e.what()); }
            continue;
            
          case h("-f"):
          case h("--camera-capture/front-camera-id"):
            try { conf.camera_capture.front_camera_id = std::stoi(*++i); }
            catch(const std::exception& e) { L(ERROR, "catch exception: " << e.what()); }
            continue;
            
          case h("-W"):
          case h("--camera-capture/width"):
            try { conf.camera_capture.width = std::stoi(*++i); }
            catch(const std::exception& e) { L(ERROR, "catch exception: " << e.what()); }
            continue;
            
          case h("-H"):
          case h("--camera-capture/height"):
            try { conf.camera_capture.height = std::stoi(*++i); }
            catch(const std::exception& e) { L(ERROR, "catch exception: " << e.what()); }
            continue;
            
          case h("-h"):
          case h("--help"):
            show_help();
            exit(0);
          case h("-v"):
          case h("--version"):
            std::cout << etupirka_t::version_string();
            exit(0);
        }
      }
      
      return conf;
    }
    
    void commandline_helper_t::show_help()
    {
      constexpr const char * const help =
        "help\n"
        "help\n"
        "help\n"
        ;
      std::cout << help;
    }
    
    void commandline_helper_t::show_default()
    {
      L(INFO, "show_default");
      show_conf(load_default());
    }
    
    void commandline_helper_t::show_conf(const configuration_t& conf)
    {
      L(INFO, "show_conf");
      
      boost::property_tree::ptree p;
      
      p.put("mode", to_string(conf.mode));
      p.put("circle_x_distance_threshold", conf.circle_x_distance_threshold);
      p.put("send_repeat_key_down_signal", conf.send_repeat_key_down_signal);
      p.put("recieve_repeat_key_down_signal", conf.recieve_repeat_key_down_signal);
      p.put("camera_capture.top_camera_id", conf.camera_capture.top_camera_id);
      p.put("camera_capture.front_camera_id", conf.camera_capture.front_camera_id);
      p.put("camera_capture.width", conf.camera_capture.width);
      p.put("camera_capture.height", conf.camera_capture.height);
      p.put("finger_detector_top.pre_bilateral_d", conf.finger_detector_top.pre_bilateral_d);
      p.put("finger_detector_top.pre_bilateral_sc", conf.finger_detector_top.pre_bilateral_sc);
      p.put("finger_detector_top.pre_bilateral_ss", conf.finger_detector_top.pre_bilateral_ss);
      p.put("finger_detector_top.pre_morphology_n", conf.finger_detector_top.pre_morphology_n);
      p.put("finger_detector_top.hsv_h_min", conf.finger_detector_top.hsv_h_min);
      p.put("finger_detector_top.hsv_h_max", conf.finger_detector_top.hsv_h_max);
      p.put("finger_detector_top.hsv_s_min", conf.finger_detector_top.hsv_s_min);
      p.put("finger_detector_top.hsv_s_max", conf.finger_detector_top.hsv_s_max);
      p.put("finger_detector_top.hsv_v_min", conf.finger_detector_top.hsv_v_min);
      p.put("finger_detector_top.hsv_v_max", conf.finger_detector_top.hsv_v_max);
      p.put("finger_detector_top.nail_morphology_n", conf.finger_detector_top.nail_morphology_n);
      p.put("finger_detector_top.nail_median_blur_ksize", conf.finger_detector_top.nail_median_blur_ksize);
      p.put("finger_detector_top.circles_dp", conf.finger_detector_top.circles_dp);
      p.put("finger_detector_top.circles_min_dist", conf.finger_detector_top.circles_min_dist);
      p.put("finger_detector_top.circles_param_1", conf.finger_detector_top.circles_param_1);
      p.put("finger_detector_top.circles_param_2", conf.finger_detector_top.circles_param_2);
      p.put("finger_detector_top.circles_min_radius", conf.finger_detector_top.circles_min_radius);
      p.put("finger_detector_top.circles_max_radius", conf.finger_detector_top.circles_max_radius);
      p.put("finger_detector_front.pre_bilateral_d", conf.finger_detector_front.pre_bilateral_d);
      p.put("finger_detector_front.pre_bilateral_sc", conf.finger_detector_front.pre_bilateral_sc);
      p.put("finger_detector_front.pre_bilateral_ss", conf.finger_detector_front.pre_bilateral_ss);
      p.put("finger_detector_front.pre_morphology_n", conf.finger_detector_front.pre_morphology_n);
      p.put("finger_detector_front.hsv_h_min", conf.finger_detector_front.hsv_h_min);
      p.put("finger_detector_front.hsv_h_max", conf.finger_detector_front.hsv_h_max);
      p.put("finger_detector_front.hsv_s_min", conf.finger_detector_front.hsv_s_min);
      p.put("finger_detector_front.hsv_s_max", conf.finger_detector_front.hsv_s_max);
      p.put("finger_detector_front.hsv_v_min", conf.finger_detector_front.hsv_v_min);
      p.put("finger_detector_front.hsv_v_max", conf.finger_detector_front.hsv_v_max);
      p.put("finger_detector_front.nail_morphology_n", conf.finger_detector_front.nail_morphology_n);
      p.put("finger_detector_front.nail_median_blur_ksize", conf.finger_detector_front.nail_median_blur_ksize);
      p.put("finger_detector_front.circles_dp", conf.finger_detector_front.circles_dp);
      p.put("finger_detector_front.circles_min_dist", conf.finger_detector_front.circles_min_dist);
      p.put("finger_detector_front.circles_param_1", conf.finger_detector_front.circles_param_1);
      p.put("finger_detector_front.circles_param_2", conf.finger_detector_front.circles_param_2);
      p.put("finger_detector_front.circles_min_radius", conf.finger_detector_front.circles_min_radius);
      p.put("finger_detector_front.circles_max_radius", conf.finger_detector_front.circles_max_radius);
      p.put("space_converter.top_camera_position", to_string(conf.space_converter.top_camera_position));
      p.put("space_converter.front_camera_position", to_string(conf.space_converter.front_camera_position));
      p.put("space_converter.top_camera_angle_x", conf.space_converter.top_camera_angle_x);
      p.put("space_converter.camera_fov_diagonal", conf.space_converter.camera_fov_diagonal);
      p.put("space_converter.camera_sensor_size", to_string(conf.space_converter.camera_sensor_size));
      p.put("space_converter.image_size", to_string(conf.space_converter.image_size));
      p.put("virtual_keyboard.database", conf.virtual_keyboard.database);
      p.put("virtual_keyboard.table", conf.virtual_keyboard.table);
      p.put("udp_sender.address", conf.udp_sender.address);
      p.put("udp_sender.port", conf.udp_sender.port);
      p.put("udp_reciever.port", conf.udp_reciever.port);
      
      boost::property_tree::write_ini(std::cout, p);
    }
    
    void commandline_helper_t::load_file(configuration_t& conf, const std::string& filename)
    {
      L(INFO, "load_file");
      std::ifstream f(filename);
      if(f)
      {
        
      }
      else
      {
        L(ERROR, "target conf file is not exists or cannot opened: " << filename);
      }
    }
    
    configuration_t commandline_helper_t::load_default()
    {
      L(INFO, "load_default");
      return
        {
          mode_t::none
        
        , {   0
          ,   1
          , 640
          , 480
          }
        
        , {  16
          ,  72
          ,  16
          
          ,   5
          
          , 315.87
          , 356.36
          ,   0.2992
          ,   0.7049
          , 120
          , 255
          
          ,   5
          ,  13
          
          ,   1
          ,   8
          , 100
          ,   8
          ,   4
          ,  12
          }
        
        , {  16
          ,  72
          ,  16
          
          ,   5
          
          , 315.87
          , 356.36
          ,   0.2992
          ,   0.7049
          , 120
          , 255
          
          ,   5
          ,  13
          
          ,   1
          ,   8
          , 100
          ,   8
          ,   4
          ,  12
          }
        
        , 6.0
        
        , { {0., 207., 264.}
          , {0.,  37., 350.}
          , 31.1
          , 64.
          , {3.60, 2.70}
          , {640, 480}
          }
        
        , { "virtual-keyboard.sqlite3"
          , "test"
          }
        
        , { "localhost"
          , 30000
          }
        
        , { 30000
          }
        
        , false
        , false
        
        , { }
        };
    }
  }
}
