#include "etupirka.hxx"

namespace arisin
{
  namespace etupirka
  {
    etupirka_t::etupirka_t(const configuration_t& conf)
      : conf_(conf)
    {
      DLOG(INFO) << "etupirka ctor";
      
#ifndef NDEBUG
      std::stringstream s;
      commandline_helper_t::show_conf(conf, s);
      DLOG(INFO) << "conf.mode: \n" << s.str();
#endif
    }
    
    void etupirka_t::run()
    {
      if(is_running_)
      {
        LOG(WARNING) << "etupirka(" << this << ") is already running, return";
        return;
      }
        
      switch(conf_.mode)
      {
        case mode_t::main:
          DLOG(INFO) << "mode is main, to run_main";
          run_main();
          return;
          
        case mode_t::reciever:
          DLOG(INFO) << "mode is reciever, to run_reciever";
          run_reciever();
          return;
          
        case mode_t::none:
        default:
          DLOG(INFO) << "mode is none, return";
          return;
      }
    }
    
    void etupirka_t::run_main()
    {
      initialize();
      
      is_running_ = true;
      
      DLOG(INFO) << "run main mode main loop";
      
      virtual_keyboard_t::pressing_keys_t pressing_keys_before;
      
      while(is_running_)
      {
        // topとfrontのカメラキャプチャー像を手に入れる。
        const auto captured_frames = (*camera_capture)();
        // topから指先群を検出する。
        const auto circles_top   = (*finger_detector_top  )(captured_frames.top  );
        // frontから指先群を検出する。
        const auto circles_front = (*finger_detector_front)(captured_frames.front);
        
        // 仮想キーボードの状態をリセット
        virtual_keyboard->reset();
            
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
          
          // 一番近い子をとりあえずcfとして迎え入れる。
          const auto& cf = *x_distance_min_element;
          
          // X座標距離に判定のしきい値を適用する
          if(d(ct[0], cf[0]) <= conf_.circle_x_distance_threshold)
          {
            // 3次元空間における座標が求まる
            const auto real_position = (*space_converter)({ct[0], ct[1] + ct[2]}, {cf[0], cf[1] + cf[2]});
            // 仮想キーボードの押下テスト＆もしかしたらシグナル追加
            virtual_keyboard->add_test(real_position[0], real_position[1], real_position[2]);
            
          }
        }
        
        // 仮想キーボードの状態を取得
        const auto pressing_keys = virtual_keyboard->pressing_keys();
        
        if(conf_.send_repeat_key_down_signal)
          // 押されているキーを全て
          for(const auto pressing_key : pressing_keys)
            // UDP送出する
            (*udp_sender)(key_signal_t(uint32_t(pressing_key), uint8_t(WonderRabbitProject::key::writer_t::state_t::down)));
        else
          // 押されているキーのうち、
          for(const auto pressing_key : pressing_keys)
            // 前回押されていなかったキーのみ
            if(std::find(std::begin(pressing_keys_before), std::end(pressing_keys_before), pressing_key) == std::end(pressing_keys_before))
              // UDP送出する
              (*udp_sender)(key_signal_t(uint32_t(pressing_key), uint8_t(WonderRabbitProject::key::writer_t::state_t::down)));
        
        // 前回のキー押下状態を全てforで回し
        for(const auto pressing_key_before : pressing_keys_before)
          // 離されたキーを検出して
          if(std::find(std::begin(pressing_keys), std::end(pressing_keys), pressing_key_before) == std::end(pressing_keys))
            // UDP送出する
            (*udp_sender)(key_signal_t(uint32_t(pressing_key_before), uint8_t(WonderRabbitProject::key::writer_t::state_t::up)));
        
        // 現在押されていたキー群を次のループでの前のキー押下状態として使えるように保存
        pressing_keys_before = pressing_keys;
      }
      
      DLOG(INFO) << "exit main loop";
    }
    
    void etupirka_t::run_reciever()
    {
      initialize();
      
      is_running_ = true;
      
      DLOG(INFO) << "run reciever mode main loop";
      
      while(is_running_)
      {
        const auto key_signal = (*udp_reciever)();
        (*key_invoker)(key_signal.code_state.code, WonderRabbitProject::key::writer_t::state_t(key_signal.code_state.state));
      }
      
      DLOG(INFO) << "exit main loop";
    }
    
    void etupirka_t::initialize()
    {
      DLOG(INFO) << "initialize";
      switch(conf_.mode)
      {
        case mode_t::main:
          camera_capture.reset(new camera_capture_t(conf_));
          finger_detector_top.reset(new finger_detector_t(conf_, true));
          finger_detector_front.reset(new finger_detector_t(conf_, false));
          space_converter.reset(new space_converter_t(conf_));
          virtual_keyboard.reset(new virtual_keyboard_t(conf_));
          udp_sender.reset(new udp_sender_t(conf_));
          udp_reciever.reset(nullptr);
          key_invoker.reset(nullptr);
          break;
          
        case mode_t::reciever:
          camera_capture.reset(nullptr);
          finger_detector_top.reset(nullptr);
          finger_detector_front.reset(nullptr);
          space_converter.reset(nullptr);
          virtual_keyboard.reset(nullptr);
          udp_sender.reset(nullptr);
          udp_reciever.reset(new udp_reciever_t(conf_));
          key_invoker.reset(new key_invoker_t(conf_));
          break;
          
        default:
          camera_capture.reset(nullptr);
          finger_detector_top.reset(nullptr);
          finger_detector_front.reset(nullptr);
          space_converter.reset(nullptr);
          virtual_keyboard.reset(nullptr);
          udp_sender.reset(nullptr);
          udp_reciever.reset(nullptr);
          key_invoker.reset(nullptr);
      }
      
      DLOG(INFO) << camera_capture.get();
      DLOG(INFO) << finger_detector_top.get();
      DLOG(INFO) << finger_detector_front.get();
      DLOG(INFO) << space_converter.get();
      DLOG(INFO) << virtual_keyboard.get();
      DLOG(INFO) << udp_sender.get();
      DLOG(INFO) << udp_reciever.get();
      DLOG(INFO) << key_invoker.get();
      
    }
    
    bool etupirka_t::is_running() const
    { return is_running_; }
  }
}