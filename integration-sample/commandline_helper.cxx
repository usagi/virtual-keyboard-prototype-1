#include "commandline_helper.hxx"

namespace
{
  constexpr size_t h(const char* const  str, int n = 0)
  { return !str[n] ? 5381 : (h(str, n+1)*33) ^ str[n]; }
  
  std::string to_string(const arisin::etupirka::mode_t m)
  {
    switch(m)
    {
      case arisin::etupirka::mode_t::none:           return "none";
      case arisin::etupirka::mode_t::main:           return "main";
      case arisin::etupirka::mode_t::reciever:       return "reciever";
      case arisin::etupirka::mode_t::main_m1:        return "main-";
      case arisin::etupirka::mode_t::reciever_p1:    return "reciever+";
      case arisin::etupirka::mode_t::dummy_main:     return "dummy-main";
      case arisin::etupirka::mode_t::dummy_reciever: return "dummy-reciever";
    }
    LOG(FATAL) << "unkown mode: " << int(m);
    throw std::runtime_error(std::string("unkown mode: ") + std::to_string(int(m)));
  }
  
  arisin::etupirka::mode_t to_mode_t(const std::string& s)
  {
    switch(h(s.data()))
    {
      case h("none"):           return arisin::etupirka::mode_t::none;
      case h("main"):           return arisin::etupirka::mode_t::main;
      case h("reciever"):       return arisin::etupirka::mode_t::reciever;
      case h("main-"):          return arisin::etupirka::mode_t::main_m1;
      case h("reciever+"):      return arisin::etupirka::mode_t::reciever_p1;
      case h("dummy-main"):     return arisin::etupirka::mode_t::dummy_main;
      case h("dummy-reciever"): return arisin::etupirka::mode_t::dummy_reciever;
    }
    LOG(FATAL) << "can not convert to mode_t from: " << s;
    throw std::runtime_error(std::string("can not convert to mode_t from: ") + s);
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
  
  template<size_t N>
  std::array<typename arisin::etupirka::configuration_t::space_converter_configuration_t::float_t, N>
  to_aNd_t(const std::string& s)
  {
    using float_t = arisin::etupirka::configuration_t::space_converter_configuration_t::float_t;
    std::vector<std::string> v;
    
    boost::split(v, s, boost::is_any_of(","));
    
    if(v.size() != N)
    {
      LOG(FATAL) << "to_aNd_t size is not equal(N=" << N << "): " << v.size();
      throw std::runtime_error(std::string("to_aNd_t size is not equal(N=") + std::to_string(N) + "): " + std::to_string(v.size()));
    }
    
    std::array<float_t, N> a;
    
    boost::transform(v, std::begin(a), [](const std::string& value){ return std::stof(value); });
    
    return a;
  }
  
}

namespace arisin
{
  namespace etupirka
  {
    configuration_t commandline_helper_t::interpret(const std::vector<std::string>& arguments)
    {
      DLOG(INFO) << "interpert";
#ifndef NDEBUG
      for(size_t n = 0; n < arguments.size(); ++n)
        DLOG(INFO) << "arguments[" << n << "]: " << arguments[n];
#endif
      
      // もし、コマンドラインに -dまたは--default-confがあればshow_defaultして終わる
      if(std::find_if(std::begin(arguments), std::end(arguments), [](const std::string& a){ return a == "-d" || a == "--default-conf"; }) != std::end(arguments))
      {
        DLOG(INFO) << "found -d(--default-conf) option in commandline arguments";
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
        LOG(WARNING) << "conf.mode=none and arguments not include -m(--mode) option. force show help and exit";
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
            catch(const std::exception& e) { LOG(ERROR) << "catch exception: " << e.what(); }
            continue;
            
          case h("-t"):
          case h("--camera-capture/top-camera-id"):
            try { conf.camera_capture.top_camera_id = std::stoi(*++i); }
            catch(const std::exception& e) { LOG(ERROR) << "catch exception: " << e.what(); }
            continue;
            
          case h("-f"):
          case h("--camera-capture/front-camera-id"):
            try { conf.camera_capture.front_camera_id = std::stoi(*++i); }
            catch(const std::exception& e) { LOG(ERROR) << "catch exception: " << e.what(); }
            continue;
            
          case h("-W"):
          case h("--camera-capture/width"):
            try { conf.camera_capture.width = std::stoi(*++i); }
            catch(const std::exception& e) { LOG(ERROR) << "catch exception: " << e.what(); }
            continue;
            
          case h("-H"):
          case h("--camera-capture/height"):
            try { conf.camera_capture.height = std::stoi(*++i); }
            catch(const std::exception& e) { LOG(ERROR) << "catch exception: " << e.what(); }
            continue;
            
          case h("--video-file-top"):
            try { conf.video_file_top = *++i; }
            catch(const std::exception& e) { LOG(ERROR) << "catch exception: " << e.what(); }
            continue;
            
          case h("--video-file-front"):
            try { conf.video_file_front = *++i; }
            catch(const std::exception& e) { LOG(ERROR) << "catch exception: " << e.what(); }
            continue;
            
          case h("-p"):
          case h("--port"):
            try { conf.udp_reciever.port = conf.udp_sender.port = std::stoi(*++i); }
            catch(const std::exception& e) { LOG(ERROR) << "catch exception: " << e.what(); }
            continue;
            
          case h("-a"):
          case h("--address"):
            try { conf.udp_sender.address = *++i; }
            catch(const std::exception& e) { LOG(ERROR) << "catch exception: " << e.what(); }
            continue;
            
          case h("-F"):
          case h("--fps"):
            try { conf.fps = std::stoi(*++i); }
            catch(const std::exception& e) { LOG(ERROR) << "catch exception: " << e.what(); }
            continue;
            
          case h("-G"):
          case h("--gui"):
            try { conf.gui = true; }
            catch(const std::exception& e) { LOG(ERROR) << "catch exception: " << e.what(); }
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
        "<Usage 0> ./etupirka [-h|--help]\n"
        "  show this help and exit.\n"
        "\n"
        "<Usage 1> ./etupirka (-v|--version)\n"
        "  show version and exit.\n"
        "\n"
        "<Usage 2> ./etupirka (-m|--mode) main [options]"
        "  run the 'main' mode.\n"
        "    * capture --> finger-detection --> space-convert\n"
        "      --> virtual-keyboard --> send-keysignal\n"
        "\n"
        "  options:\n"
        "    [-t|--camera-capture/top-camera-id] (camera_device_id:int)\n"
        "      set top-cam device id to (camera_device_id:int).\n"
        "\n"
        "    [-f|--camera-capture/front-camera-id] (camera_device_id:int)\n"
        "      set front-cam device id to (camera_device_id:int).\n"
        "\n"
        "    [-W|--camera-capture/width] (camera-resolution-width:int)\n"
        "      set camera resolution width to (camera-resolution-with:int).\n"
        "\n"
        "    [-H|--camera-capture/height] (camera-resolution-height:int)\n"
        "      set camera resolution height to (camera-resolution-height:int).\n"
        "\n"
        "    [-p|--port] (port:int)\n"
        "      set send/recieve port number to (port:int).\n"
        "\n"
        "    [-a|--address] (address:string)\n"
        "      set send-to address to (address:string).\n"
        "\n"
        "    [-F|--fps] (fps:int)\n"
        "      set main-loop fps[frames/sec] to (fps:int).\n"
        "\n"
        "    [--video-file-top] (filename:string)"
        "      set top-cam source to video file (filename:string)."
        "\n"
        "    [--video-file-front] (filename:string)"
        "      set front-cam source to video file (filename:string)."
        "\n"
        "<Usage 3> ./etupirka (-m|--mode) reciever [options]"
        "  run the 'main' mode.\n"
        "    * recieve-keysignal --> invoke-keysignal\n"
        "\n"
        "  options:\n"
        "    [-p|--port] (port:int)\n"
        "      set send/recieve port number to (port:int).\n"
        "\n"
        "    [-F|--fps] (fps:int)\n"
        "      set main-loop fps[frames/sec] to (fps:int).\n"
        "\n"
        "    [-G|--gui]\n"
        "      set enable GUI.\n"
        "\n"
        "<Usage 4> ./etupirka (-d|--default-conf)\n"
        "  show etupirka default configuration and exit.\n"
        "  if you want save to file: `./etupirka -d > etupirka.conf`"
        "\n"
        "<note>\n"
        "  etupirka load configuration priority is: \n"
        "    (low-priority)"
        "    0: default\n"
        "    1: etupirka.conf\n"
        "    2: commandline options\n"
        "    (high-priority)\n"
        ;
      std::cout << help;
    }
    
    void commandline_helper_t::show_default()
    {
      DLOG(INFO) << "show_default";
      show_conf(load_default());
    }
    
    void commandline_helper_t::save_conf(const configuration_t& conf, const std::string& filename)
    {
      DLOG(INFO) << "save_conf";
      boost::property_tree::write_ini(filename, boost_ptree(conf));
    }
    
    void commandline_helper_t::show_conf(const configuration_t& conf, std::ostream& out)
    {
      DLOG(INFO) << "show_conf";
      boost::property_tree::write_ini(out, boost_ptree(conf));
    }
    
    boost::property_tree::ptree commandline_helper_t::boost_ptree(const configuration_t& conf)
    {
      boost::property_tree::ptree p;
      
      p.put("mode", to_string(conf.mode));
      p.put("gui", conf.gui);
      p.put("fps", conf.fps);
      p.put("video_file_top", conf.video_file_top);
      p.put("video_file_front", conf.video_file_front);
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
      
      return p;
    }
    
    void commandline_helper_t::load_file(configuration_t& conf, const std::string& filename)
    {
      DLOG(INFO) << "load_file";
      
      boost::property_tree::ptree p;
      
      try
      { read_ini(filename, p); }
      catch(...)
      {
        LOG(ERROR) << "exception: boost::property_tree::read_ini; file " << filename << " is not exists, maybe.";
        return;
      }
      
      if(const auto v = p.get_optional<std::string>("mode")) conf.mode = to_mode_t(v.get());
#define ARISIN_ETUPIRKA_TMP(T,N) \
      if(const auto v = p.get_optional<T>( #N )) conf. N = v.get();
      ARISIN_ETUPIRKA_TMP(int, fps)
      ARISIN_ETUPIRKA_TMP(bool, gui)
      ARISIN_ETUPIRKA_TMP(std::string, video_file_top)
      ARISIN_ETUPIRKA_TMP(std::string, video_file_front)
      ARISIN_ETUPIRKA_TMP(float, circle_x_distance_threshold)
      ARISIN_ETUPIRKA_TMP(bool, send_repeat_key_down_signal)
      ARISIN_ETUPIRKA_TMP(bool, recieve_repeat_key_down_signal)
      
      ARISIN_ETUPIRKA_TMP(double, camera_capture.top_camera_id)
      ARISIN_ETUPIRKA_TMP(double, camera_capture.front_camera_id)
      ARISIN_ETUPIRKA_TMP(double, camera_capture.width)
      ARISIN_ETUPIRKA_TMP(double, camera_capture.height)
      
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.pre_bilateral_d)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.pre_bilateral_sc)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.pre_bilateral_ss)
      ARISIN_ETUPIRKA_TMP(int, finger_detector_top.pre_morphology_n)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.hsv_h_min)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.hsv_h_max)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.hsv_s_min)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.hsv_s_max)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.hsv_v_min)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.hsv_v_max)
      ARISIN_ETUPIRKA_TMP(int, finger_detector_top.nail_morphology_n)
      ARISIN_ETUPIRKA_TMP(int, finger_detector_top.nail_median_blur_ksize)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.circles_dp)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.circles_min_dist)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.circles_param_1)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_top.circles_param_2)
      ARISIN_ETUPIRKA_TMP(int, finger_detector_top.circles_min_radius)
      ARISIN_ETUPIRKA_TMP(int, finger_detector_top.circles_max_radius)

      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.pre_bilateral_d)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.pre_bilateral_sc)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.pre_bilateral_ss)
      ARISIN_ETUPIRKA_TMP(int, finger_detector_front.pre_morphology_n)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.hsv_h_min)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.hsv_h_max)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.hsv_s_min)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.hsv_s_max)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.hsv_v_min)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.hsv_v_max)
      ARISIN_ETUPIRKA_TMP(int, finger_detector_front.nail_morphology_n)
      ARISIN_ETUPIRKA_TMP(int, finger_detector_front.nail_median_blur_ksize)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.circles_dp)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.circles_min_dist)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.circles_param_1)
      ARISIN_ETUPIRKA_TMP(double, finger_detector_front.circles_param_2)
      ARISIN_ETUPIRKA_TMP(int, finger_detector_front.circles_min_radius)
      ARISIN_ETUPIRKA_TMP(int, finger_detector_front.circles_max_radius)

      if(const auto v = p.get_optional<std::string>("space_converter.top_camera_position")) conf.space_converter.top_camera_position = to_aNd_t<3>(v.get());
      if(const auto v = p.get_optional<std::string>("space_converter.front_camera_position")) conf.space_converter.front_camera_position = to_aNd_t<3>(v.get());
      ARISIN_ETUPIRKA_TMP(float, space_converter.top_camera_angle_x)
      ARISIN_ETUPIRKA_TMP(float, space_converter.camera_fov_diagonal)
      if(const auto v = p.get_optional<std::string>("space_converter.camera_sensor_size")) conf.space_converter.camera_sensor_size = to_aNd_t<2>(v.get());
      if(const auto v = p.get_optional<std::string>("space_converter.image_size")) conf.space_converter.image_size = to_aNd_t<2>(v.get());
      
      ARISIN_ETUPIRKA_TMP(std::string, virtual_keyboard.database)
      ARISIN_ETUPIRKA_TMP(std::string, virtual_keyboard.table)
      
      ARISIN_ETUPIRKA_TMP(std::string, udp_sender.address)
      ARISIN_ETUPIRKA_TMP(int, udp_sender.port)
      ARISIN_ETUPIRKA_TMP(int, udp_reciever.port)
#undef ARISIN_ETUPIRKA_TMP
    }
    
    configuration_t commandline_helper_t::load_default()
    {
      DLOG(INFO) << "load_default";
      return
        {
          mode_t::none
        
        , false
        
        , 30
        
        , ""
        , ""
        
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
          
          , 356.33
          , 390
          ,   0.1105
          ,   0.3118
          , 199
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
        
        , { {{0., 207., 264.}}
          , {{0.,  37., 350.}}
          , 31.1
          , 64.
          , {{3.60, 2.70}}
          , {{640, 480}}
          }
        
        , { "virtual-keyboard.sqlite3"
          , "test"
          }
        
        , { "127.0.0.1"
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
