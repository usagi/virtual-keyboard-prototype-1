#include <thread>
#include <future>
#include <boost/version.hpp>
#include <boost/chrono.hpp>
#include "etupirka.hxx"

namespace
{
  inline void adjust_fps(const std::function<void()>& f, const std::chrono::nanoseconds& target_wait)
  {
    const auto time_start = std::chrono::high_resolution_clock::now();
    
    f();
    
    const auto time_delta = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now() - time_start );
    DLOG(INFO) << "time_delta [ms]: " << float(time_delta.count()) / 1000000;
    const auto time_wait = target_wait - time_delta;
    if(time_wait.count() > 0)
    {
      DLOG(INFO) << "time_wait [ms]: " << float(time_wait.count()) / 1000000;
#if !defined(__clang__) && __GNUC__ == 4 &&  __GNUC_MINOR__ < 8
      const auto& time_wait_boost = *reinterpret_cast<const boost::chrono::nanoseconds*>(&time_wait);
  #if BOOST_VERSION >= 105000
      boost::this_thread::sleep_for(time_wait_boost);
  #else
      const auto time_wait_boost_posix = boost::posix_time::microseconds(boost::chrono::duration_cast<boost::chrono::microseconds>(time_wait_boost).count());
      boost::this_thread::sleep(time_wait_boost_posix);
  #endif
#else
      std::this_thread::sleep_for(time_wait);
#endif
    }
  }
}

namespace arisin
{
  namespace etupirka
  {
    etupirka_t::etupirka_t(const configuration_t& conf)
      : conf_(conf)
      , main_loop_wait_( std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::seconds(1) / static_cast<long double>(conf.fps) ) )
    {
      DLOG(INFO) << "etupirka ctor";
      
#ifndef NDEBUG
      std::stringstream s;
      commandline_helper_t::show_conf(conf, s);
      DLOG(INFO) << "conf.mode: \n" << s.str();
#endif
      
      DLOG(INFO) << "main_loop_wait[ns]: " << main_loop_wait_.count();
    }
    
    void etupirka_t::run()
    {
      if(is_running_)
      {
        LOG(WARNING) << "etupirka(" << this << ") is already running, return";
        return;
      }
      
      auto t = std::thread([&]()
      {
        std::string buffer;
        std::getline(std::cin, buffer);
        is_running_ = false;
      });
      
      switch(conf_.mode)
      {
        case mode_t::main:
          DLOG(INFO) << "mode is main, to run_main";
          run_main();
          break;
          
        case mode_t::reciever:
          DLOG(INFO) << "mode is reciever, to run_reciever";
          run_reciever();
          break;
          
        case mode_t::main_m1:
          DLOG(INFO) << "mode is main-, to run_main";
          run_main_m1();
          break;
          
        case mode_t::reciever_p1:
          DLOG(INFO) << "mode is reciever+, to run_reciever";
          run_reciever_p1();
          break;
          
        case mode_t::dummy_main:
          DLOG(INFO) << "mode is dummy-main, to run_main";
          run_dummy_main();
          break;
          
        case mode_t::dummy_reciever:
          DLOG(INFO) << "mode is dummy-reciever, to run_reciever";
          run_dummy_reciever();
          break;
          
        case mode_t::none:
        default:
          DLOG(INFO) << "mode is none, return";
      }
      
      t.join();
      
      DLOG(INFO) << "exit main loop";
    }
    
    void etupirka_t::run_main()
    {
      DLOG(INFO) << "to initialize";
      initialize();
      
      is_running_ = true;
      
      DLOG(INFO) << "run main mode main loop";
      
      virtual_keyboard_t::pressing_keys_t pressing_keys_before;
      
      while(is_running_)
      {
        adjust_fps([&]()
        {
          DLOG(INFO) << "to camera_capture()";
          // topとfrontのカメラキャプチャー像を手に入れる。
          const auto captured_frames = (*camera_capture)();
          
          if(captured_frames.top.rows != conf_.camera_capture.height || captured_frames.top.cols != conf_.camera_capture.width)
          {
            LOG(WARNING) << "top-cam captured frame is invalid data; skip the frame and continue";
            return;
          }
          
          if(captured_frames.front.rows != conf_.camera_capture.height || captured_frames.front.cols != conf_.camera_capture.width)
          {
            LOG(WARNING) << "front-cam captured frame is invalid data; skip the frame and continue";
            return;
          }
          
          DLOG(INFO) << "to finger_detector_top()";
          // topから指先群を検出する。
          auto finger_detector_future_top = std::async([&](){ return (*finger_detector_top)(captured_frames.top); });
          
          DLOG(INFO) << "to finger_detector_front()";
          // frontから指先群を検出する。
          auto finger_detector_future_front = std::async([&](){ return (*finger_detector_front)(captured_frames.front); });
          
          const auto circles_top   = finger_detector_future_top.get();
          const auto circles_front = finger_detector_future_front.get();
          DLOG(INFO) << "circles_top.size(): "   << circles_top.size();
          DLOG(INFO) << "circles_front.size(): " << circles_front.size();
          
          DLOG(INFO) << "to virtual_keyboard->reset()";
          // 仮想キーボードの状態をリセット
          virtual_keyboard->reset();
              
          DLOG(INFO) << "to for(circles_top)";
          // topの検出円群をforで回す
          for(const auto& ct : circles_top)
          {
            // ここでだけ何度も使うので2実数点の距離を算出するλ式にdと名づけて定義しておく。
            auto d = [](float a, float b){ return std::abs(a - b); };
            
            // 着目しているtopのある検出円のX座標に最も近いX座標のfrontの検出円を探索する。
            auto x_distance_min_element = std::min_element
            ( std::begin(circles_front), std::end(circles_front)
            , [&](const finger_detector_t::circles_t::value_type& cf1, const finger_detector_t::circles_t::value_type& cf2)
              { return d(ct[0], cf1[0]) < d(ct[0], cf2[0]); }
            );
            
            if(x_distance_min_element == std::end(circles_front))
              continue;
            
            // 一番近い子をとりあえずcfとして迎え入れる。
            const auto& cf = *x_distance_min_element;
            DLOG(INFO) << "x-distance(ct, cf): " << d(ct[0], cf[0]);
            
            // X座標距離に判定のしきい値を適用する
            if(d(ct[0], cf[0]) <= conf_.circle_x_distance_threshold)
            {
              // 3次元空間における座標が求まる
              const auto real_position = (*space_converter)({{ct[0], ct[1] + ct[2]}}, {{cf[0], cf[1] + cf[2]}});
              DLOG(INFO) << "estimated real_position: (" << real_position[0] << "," << real_position[1] << "," << real_position[2] << ")";
              DLOG(INFO) << "to virtual_keyboard->add_test()";
              // 仮想キーボードの押下テスト＆もしかしたらシグナル追加
              virtual_keyboard->add_test(real_position[0], real_position[1], real_position[2]);
            }
          }
          
          DLOG(INFO) << "to virtual_keyboard->pressing_keys()";
          // 仮想キーボードの状態を取得
          const auto pressing_keys = virtual_keyboard->pressing_keys();
          
          if(conf_.send_repeat_key_down_signal)
          {
            DLOG(INFO) << "to send key-down all";
            // 押されているキーを全て
            for(const auto pressing_key : pressing_keys)
            {
              DLOG(INFO) << "to udp_sender(); key-down signal: " << pressing_key;
              // UDP送出する
              (*udp_sender)(key_signal_t(uint32_t(pressing_key), uint8_t(WonderRabbitProject::key::writer_t::state_t::down)));
            }
          }
          else
          {
            DLOG(INFO) << "to send key-down without before downed";
            // 押されているキーのうち、
            for(const auto pressing_key : pressing_keys)
              // 前回押されていなかったキーのみ
              if(std::find(std::begin(pressing_keys_before), std::end(pressing_keys_before), pressing_key) == std::end(pressing_keys_before))
              {
                DLOG(INFO) << "to udp_sender(); key-down signal: " << pressing_key;
                // UDP送出する
                (*udp_sender)(key_signal_t(uint32_t(pressing_key), uint8_t(WonderRabbitProject::key::writer_t::state_t::down)));
              }
          }
          
          DLOG(INFO) << "to send key-up";
          // 前回のキー押下状態を全てforで回し
          for(const auto pressing_key_before : pressing_keys_before)
            // 離されたキーを検出して
            if(std::find(std::begin(pressing_keys), std::end(pressing_keys), pressing_key_before) == std::end(pressing_keys))
            {
              DLOG(INFO) << "to udp_sender(); key-up signal: " << pressing_key_before;
              // UDP送出する
              (*udp_sender)(key_signal_t(uint32_t(pressing_key_before), uint8_t(WonderRabbitProject::key::writer_t::state_t::up)));
            }
          
          // 現在押されていたキー群を次のループでの前のキー押下状態として使えるように保存
          pressing_keys_before = pressing_keys;
          
          if(conf_.gui)
          {
            DLOG(INFO) << "gui()";
            (*gui)({captured_frames.top, captured_frames.front, finger_detector_top->effected_frame(), finger_detector_front->effected_frame(), circles_top, circles_front});
            
            DLOG(INFO) << "propagate conf to finger_detector_top/front";
            if(gui->current_is_top())
            {
              DLOG(INFO) << "set to top";
              finger_detector_top->set(gui->current_finger_detector_conf());
            }
            else
            {
              DLOG(INFO) << "set to front";
              finger_detector_front->set(gui->current_finger_detector_conf());
            }
          }
        }
        , main_loop_wait_
        );
      }
    }
    
    void etupirka_t::run_reciever()
    {
      initialize();
      
      is_running_ = true;
      
      DLOG(INFO) << "run reciever mode main loop";
      
      while(is_running_)
      {
        adjust_fps([&]()
        {
          const auto key_signal = (*udp_reciever)();
          (*key_invoker)(key_signal.code_state.code, WonderRabbitProject::key::writer_t::state_t(key_signal.code_state.state));
        }
        , main_loop_wait_
        );
      }
    }
    
    void etupirka_t::run_main_m1()
    {
      DLOG(INFO) << "to initialize";
      initialize();
      
      is_running_ = true;
      
      DLOG(INFO) << "run main- mode main loop";
      
      while(is_running_)
      {
        adjust_fps([&]()
        {
          DLOG(INFO) << "to camera_capture()";
          // topとfrontのカメラキャプチャー像を手に入れる。
          const auto captured_frames = (*camera_capture)();
          
          if(captured_frames.top.rows != conf_.camera_capture.height || captured_frames.top.cols != conf_.camera_capture.width)
          {
            LOG(WARNING) << "top-cam captured frame is invalid data; skip the frame and continue";
            return;
          }
          
          if(captured_frames.front.rows != conf_.camera_capture.height || captured_frames.front.cols != conf_.camera_capture.width)
          {
            LOG(WARNING) << "front-cam captured frame is invalid data; skip the frame and continue";
            return;
          }
          
          (*udp_sender)(captured_frames);
        }
        , main_loop_wait_
        );
      }
    }
    
    void etupirka_t::run_reciever_p1()
    {
      initialize();
      
      virtual_keyboard_t::pressing_keys_t pressing_keys_before;
      
      is_running_ = true;
      
      DLOG(INFO) << "run reciever+ mode main loop";
      
      while(is_running_)
      {
        adjust_fps([&]()
        {
          const auto captured_frames = udp_reciever->recieve_captured_frames();
          
          DLOG(INFO) << "to finger_detector_top()";
          // topから指先群を検出する。
          auto finger_detector_future_top = std::async([&](){ return (*finger_detector_top)(captured_frames.top); });
          
          DLOG(INFO) << "to finger_detector_front()";
          // frontから指先群を検出する。
          auto finger_detector_future_front = std::async([&](){ return (*finger_detector_front)(captured_frames.front); });
          
          const auto circles_top   = finger_detector_future_top.get();
          const auto circles_front = finger_detector_future_front.get();
          DLOG(INFO) << "circles_top.size(): "   << circles_top.size();
          DLOG(INFO) << "circles_front.size(): " << circles_front.size();
          
          DLOG(INFO) << "to virtual_keyboard->reset()";
          // 仮想キーボードの状態をリセット
          virtual_keyboard->reset();
              
          DLOG(INFO) << "to for(circles_top)";
          // topの検出円群をforで回す
          for(const auto& ct : circles_top)
          {
            // ここでだけ何度も使うので2実数点の距離を算出するλ式にdと名づけて定義しておく。
            auto d = [](float a, float b){ return std::abs(a - b); };
            
            // 着目しているtopのある検出円のX座標に最も近いX座標のfrontの検出円を探索する。
            auto x_distance_min_element = std::min_element
            ( std::begin(circles_front), std::end(circles_front)
            , [&](const finger_detector_t::circles_t::value_type& cf1, const finger_detector_t::circles_t::value_type& cf2)
              { return d(ct[0], cf1[0]) < d(ct[0], cf2[0]); }
            );
            
            if(x_distance_min_element == std::end(circles_front))
              continue;
            
            // 一番近い子をとりあえずcfとして迎え入れる。
            const auto& cf = *x_distance_min_element;
            DLOG(INFO) << "x-distance(ct, cf): " << d(ct[0], cf[0]);
            
            // X座標距離に判定のしきい値を適用する
            if(d(ct[0], cf[0]) <= conf_.circle_x_distance_threshold)
            {
              // 3次元空間における座標が求まる
              const auto real_position = (*space_converter)({{ct[0], ct[1] + ct[2]}}, {{cf[0], cf[1] + cf[2]}});
              DLOG(INFO) << "estimated real_position: (" << real_position[0] << "," << real_position[1] << "," << real_position[2] << ")";
              DLOG(INFO) << "to virtual_keyboard->add_test()";
              // 仮想キーボードの押下テスト＆もしかしたらシグナル追加
              virtual_keyboard->add_test(real_position[0], real_position[1], real_position[2]);
            }
          }
          
          DLOG(INFO) << "to virtual_keyboard->pressing_keys()";
          // 仮想キーボードの状態を取得
          const auto pressing_keys = virtual_keyboard->pressing_keys();
          
          if(conf_.send_repeat_key_down_signal)
          {
            DLOG(INFO) << "to send key-down all";
            // 押されているキーを全て
            for(const auto pressing_key : pressing_keys)
            {
              DLOG(INFO) << "to udp_sender(); key-down signal: " << pressing_key;
              // キーストローク送出する
              (*key_invoker)(pressing_key, WonderRabbitProject::key::writer_t::state_t::down);
            }
          }
          else
          {
            DLOG(INFO) << "to send key-down without before downed";
            // 押されているキーのうち、
            for(const auto pressing_key : pressing_keys)
              // 前回押されていなかったキーのみ
              if(std::find(std::begin(pressing_keys_before), std::end(pressing_keys_before), pressing_key) == std::end(pressing_keys_before))
              {
                DLOG(INFO) << "to udp_sender(); key-down signal: " << pressing_key;
                // キーストローク送出する
                (*key_invoker)(pressing_key, WonderRabbitProject::key::writer_t::state_t::down);
              }
          }
          
          DLOG(INFO) << "to send key-up";
          // 前回のキー押下状態を全てforで回し
          for(const auto pressing_key_before : pressing_keys_before)
            // 離されたキーを検出して
            if(std::find(std::begin(pressing_keys), std::end(pressing_keys), pressing_key_before) == std::end(pressing_keys))
            {
              DLOG(INFO) << "to udp_sender(); key-up signal: " << pressing_key_before;
              // キーストローク送出する
              (*key_invoker)(pressing_key_before, WonderRabbitProject::key::writer_t::state_t::up);
            }
          
          // 現在押されていたキー群を次のループでの前のキー押下状態として使えるように保存
          pressing_keys_before = pressing_keys;
          
          if(conf_.gui)
          {
            DLOG(INFO) << "gui()";
            (*gui)({captured_frames.top, captured_frames.front, finger_detector_top->effected_frame(), finger_detector_front->effected_frame(), circles_top, circles_front});
            
            DLOG(INFO) << "propagate conf to finger_detector_top/front";
            if(gui->current_is_top())
            {
              DLOG(INFO) << "set to top";
              finger_detector_top->set(gui->current_finger_detector_conf());
            }
            else
            {
              DLOG(INFO) << "set to front";
              finger_detector_front->set(gui->current_finger_detector_conf());
            }
          }
          
        }
        , main_loop_wait_
        );
      }
    }
    
    void etupirka_t::run_dummy_main()
    {
      DLOG(INFO) << "to initialize";
      initialize();
      
      is_running_ = true;
      
      DLOG(INFO) << "run dummy-main mode main loop";
      
      std::mt19937 rng;
      std::uniform_int_distribution<uint32_t> distribution(4, 40);
      
      while(is_running_)
      {
        adjust_fps([&]()
        {
          (*udp_sender)(key_signal_t(distribution(rng), uint8_t(WonderRabbitProject::key::writer_t::state_t::press)));
        }
        , main_loop_wait_
        );
      }
    }
    
    void etupirka_t::run_dummy_reciever()
    {
      DLOG(INFO) << "to initialize";
      initialize();
      
      is_running_ = true;
      
      DLOG(INFO) << "run dummy-reciever mode main loop";
      
      std::mt19937 rng;
      std::uniform_int_distribution<uint32_t> distribution(4, 40);
      
      while(is_running_)
      {
        adjust_fps([&]()
        {
          (*key_invoker)(distribution(rng), WonderRabbitProject::key::writer_t::state_t::press);
        }
        , main_loop_wait_
        );
      }
    }
    
    void etupirka_t::initialize()
    {
      DLOG(INFO) << "initialize";
      switch(conf_.mode)
      {
        case mode_t::main:
          DLOG(INFO) << "to initialize camera_capture";
          camera_capture.reset(new camera_capture_t(conf_));
          DLOG(INFO) << "to initialize finger_detector_top";
          finger_detector_top.reset(new finger_detector_t(conf_, true));
          DLOG(INFO) << "to initialize finger_detector_front";
          finger_detector_front.reset(new finger_detector_t(conf_, false));
          DLOG(INFO) << "to initialize space_converter";
          space_converter.reset(new space_converter_t(conf_));
          DLOG(INFO) << "to initialize virtual_keyboard";
          virtual_keyboard.reset(new virtual_keyboard_t(conf_));
          DLOG(INFO) << "to initialize udp_sender";
          udp_sender.reset(new udp_sender_t(conf_));
          DLOG(INFO) << "to nullptr udp_reciever";
          udp_reciever.reset(nullptr);
          DLOG(INFO) << "to nullptr key_invoker";
          key_invoker.reset(nullptr);
          DLOG(INFO) << "to " << ( conf_.gui ? "initialize" : "nullptr" ) << " gui";
          gui.reset( conf_.gui ? new gui_t(conf_) : nullptr);
          break;
          
        case mode_t::reciever:
          DLOG(INFO) << "to nullptr camera_capture";
          camera_capture.reset(nullptr);
          DLOG(INFO) << "to nullptr finger_detector_top";
          finger_detector_top.reset(nullptr);
          DLOG(INFO) << "to nullptr finger_detector_front";
          finger_detector_front.reset(nullptr);
          DLOG(INFO) << "to nullptr space_converter";
          space_converter.reset(nullptr);
          DLOG(INFO) << "to nullptr virtual_keyboard";
          virtual_keyboard.reset(nullptr);
          DLOG(INFO) << "to nullptr udp_sender";
          udp_sender.reset(nullptr);
          DLOG(INFO) << "to initialize udp_reciever";
          udp_reciever.reset(new udp_reciever_t(conf_));
          DLOG(INFO) << "to initialize key_invoker";
          key_invoker.reset(new key_invoker_t(conf_));
          DLOG(INFO) << "to nullptr gui";
          gui.reset(nullptr);
          break;
          
        case mode_t::main_m1:
          DLOG(INFO) << "to initialize camera_capture";
          camera_capture.reset(new camera_capture_t(conf_));
          DLOG(INFO) << "to nullptr finger_detector_top";
          finger_detector_top.reset(nullptr);
          DLOG(INFO) << "to nullptr finger_detector_front";
          finger_detector_front.reset(nullptr);
          DLOG(INFO) << "to nullptr space_converter";
          space_converter.reset(nullptr);
          DLOG(INFO) << "to nullptr virtual_keyboard";
          virtual_keyboard.reset(nullptr);
          DLOG(INFO) << "to nullptr udp_sender";
          udp_sender.reset(nullptr);
          DLOG(INFO) << "to initialize udp_sender";
          udp_sender.reset(new udp_sender_t(conf_));
          DLOG(INFO) << "to nullptr udp_reciever";
          udp_reciever.reset(nullptr);
          DLOG(INFO) << "to nullptr key_invoker";
          key_invoker.reset(nullptr);
          DLOG(INFO) << "to nullptr gui";
          gui.reset(nullptr);
          break;
          
        case mode_t::reciever_p1:
          DLOG(INFO) << "to nullptr camera_capture";
          camera_capture.reset(nullptr);
          DLOG(INFO) << "to initialize finger_detector_top";
          finger_detector_top.reset(new finger_detector_t(conf_, true));
          DLOG(INFO) << "to initialize finger_detector_front";
          finger_detector_front.reset(new finger_detector_t(conf_, false));
          DLOG(INFO) << "to initialize space_converter";
          space_converter.reset(new space_converter_t(conf_));
          DLOG(INFO) << "to initialize virtual_keyboard";
          virtual_keyboard.reset(new virtual_keyboard_t(conf_));
          DLOG(INFO) << "to initialize udp_reciever";
          udp_reciever.reset(new udp_reciever_t(conf_));
          DLOG(INFO) << "to initialize key_invoker";
          key_invoker.reset(new key_invoker_t(conf_));
          DLOG(INFO) << "to " << ( conf_.gui ? "initialize" : "nullptr" ) << " gui";
          gui.reset( conf_.gui ? new gui_t(conf_) : nullptr);
          break;
          
        case mode_t::dummy_main:
          DLOG(INFO) << "to nullptr camera_capture";
          camera_capture.reset(nullptr);
          DLOG(INFO) << "to nullptr finger_detector_top";
          finger_detector_top.reset(nullptr);
          DLOG(INFO) << "to nullptr finger_detector_front";
          finger_detector_front.reset(nullptr);
          DLOG(INFO) << "to nullptr space_converter";
          space_converter.reset(nullptr);
          DLOG(INFO) << "to nullptr virtual_keyboard";
          virtual_keyboard.reset(nullptr);
          DLOG(INFO) << "to initialize udp_sender";
          udp_sender.reset(new udp_sender_t(conf_));
          DLOG(INFO) << "to nullptr udp_reciever";
          udp_reciever.reset(nullptr);
          DLOG(INFO) << "to nullptr key_invoker";
          key_invoker.reset(nullptr);
          DLOG(INFO) << "to nullptr gui";
          gui.reset(nullptr);
          break;
          
        case mode_t::dummy_reciever:
          DLOG(INFO) << "to nullptr camera_capture";
          camera_capture.reset(nullptr);
          DLOG(INFO) << "to nullptr finger_detector_top";
          finger_detector_top.reset(nullptr);
          DLOG(INFO) << "to nullptr finger_detector_front";
          finger_detector_front.reset(nullptr);
          DLOG(INFO) << "to nullptr space_converter";
          space_converter.reset(nullptr);
          DLOG(INFO) << "to nullptr virtual_keyboard";
          virtual_keyboard.reset(nullptr);
          DLOG(INFO) << "to nullptr udp_sender";
          udp_sender.reset(nullptr);
          DLOG(INFO) << "to nullptr udp_reciever";
          udp_reciever.reset(nullptr);
          DLOG(INFO) << "to initialize key_invoker";
          key_invoker.reset(new key_invoker_t(conf_));
          DLOG(INFO) << "to nullptr gui";
          gui.reset(nullptr);
          break;
          
        default:
          DLOG(INFO) << "to nullptr camera_capture";
          camera_capture.reset(nullptr);
          DLOG(INFO) << "to nullptr finger_detector_top";
          finger_detector_top.reset(nullptr);
          DLOG(INFO) << "to nullptr finger_detector_front";
          finger_detector_front.reset(nullptr);
          DLOG(INFO) << "to nullptr space_converter";
          space_converter.reset(nullptr);
          DLOG(INFO) << "to nullptr virtual_keyboard";
          virtual_keyboard.reset(nullptr);
          DLOG(INFO) << "to nullptr udp_sender";
          udp_sender.reset(nullptr);
          DLOG(INFO) << "to nullptr udp_reciever";
          udp_reciever.reset(nullptr);
          DLOG(INFO) << "to nullptr key_invoker";
          key_invoker.reset(nullptr);
          DLOG(INFO) << "to nullptr gui";
          gui.reset(nullptr);
      }

      DLOG(INFO) << "done initialize all submodules";
      
      DLOG(INFO) << "camera-capture address       : " << camera_capture.get();
      DLOG(INFO) << "finger-detector-top address  : " << finger_detector_top.get();
      DLOG(INFO) << "finger-detector-front address: " << finger_detector_front.get();
      DLOG(INFO) << "space-converter address      : " << space_converter.get();
      DLOG(INFO) << "virtual-keyboard address     : " << virtual_keyboard.get();
      DLOG(INFO) << "udp-sender address           : " << udp_sender.get();
      DLOG(INFO) << "udp-reciever address         : " << udp_reciever.get();
      DLOG(INFO) << "key-invoker address          : " << key_invoker.get();
      DLOG(INFO) << "gui address                  : " << gui.get();
    }
    
    bool etupirka_t::is_running() const
    { return is_running_; }
  }
}
