#include "gui.hxx"
#include "cv_gui_helper.hxx"
#include "commandline_helper.hxx"

namespace arisin
{
  namespace etupirka
  {
    gui_t::gui_t(configuration_t& conf)
      : conf_(conf)
      , current_finger_detector_conf_(conf.finger_detector_top)
      , prev_top_front_switch(0)
    {
      DLOG(INFO) << "ctor start";
      
      auto& cv_gui_helper = cv_gui_helper_t::instance();
      
      DLOG(INFO) << "cv_gui_helper address: " << &cv_gui_helper;
      
      DLOG(INFO) << "new_windows";
      
      cv_gui_helper.new_windows
      ( cv_gui_helper.make_new_window_params( window::in_top      , "top-cam(input)"   , CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
      , cv_gui_helper.make_new_window_params( window::in_front    , "front-cam(input)" , CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
      , cv_gui_helper.make_new_window_params( window::out_top     , "top-cam(output)"  , CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
      , cv_gui_helper.make_new_window_params( window::out_front   , "front-cam(output)", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
      , cv_gui_helper.make_new_window_params( window::controller_1, "controller 1"     , CV_WINDOW_NORMAL | CV_WINDOW_FREERATIO | CV_GUI_EXPANDED)
      , cv_gui_helper.make_new_window_params( window::controller_2, "controller 2"     , CV_WINDOW_NORMAL | CV_WINDOW_FREERATIO | CV_GUI_EXPANDED)
      );
      
      DLOG(INFO) << "resize in_top";
      cv_gui_helper.resize(window::in_top   , conf.camera_capture.width, conf.camera_capture.height);
      DLOG(INFO) << "resize in_front";
      cv_gui_helper.resize(window::in_front , conf.camera_capture.width, conf.camera_capture.height);
      DLOG(INFO) << "resize out_top";
      cv_gui_helper.resize(window::out_top  , conf.camera_capture.width, conf.camera_capture.height);
      DLOG(INFO) << "resize out_front";
      cv_gui_helper.resize(window::out_front, conf.camera_capture.width, conf.camera_capture.height);
      
      DLOG(INFO) << "move in_top";
      cv_gui_helper.move(window::in_top      ,    0,   0);
      DLOG(INFO) << "move in_front";
      cv_gui_helper.move(window::in_front    ,    0, 480);
      DLOG(INFO) << "move out_top";
      cv_gui_helper.move(window::out_top     ,  640,   0);
      DLOG(INFO) << "move out_front";
      cv_gui_helper.move(window::out_front   ,  640, 480);
      DLOG(INFO) << "move controller_1";
      cv_gui_helper.move(window::controller_1, 1280,   0);
      DLOG(INFO) << "move controller_2";
      cv_gui_helper.move(window::controller_2, 1600,   0);
      
      DLOG(INFO) << "new_trackbars";
      cv_gui_helper.new_trackbars
      ( cv_gui_helper.make_new_trackbar_params( trackbar::top_front_switch      , "0:top/1:front switch"        , window::controller_1,   0,   1)
      , cv_gui_helper.make_new_trackbar_params( trackbar::save                  , "1:save"                      , window::controller_1,   0,   1)
      , cv_gui_helper.make_new_trackbar_params( trackbar::load                  , "1:load"                      , window::controller_1,   0,   1)
      , cv_gui_helper.make_new_trackbar_params( trackbar::diameter              , "bilateral diameter    (x100)", window::controller_1, current_finger_detector_conf_.pre_bilateral_d, 127, 100)
      , cv_gui_helper.make_new_trackbar_params( trackbar::sigma_color           , "bilateral sigma color (x100)", window::controller_1, current_finger_detector_conf_.pre_bilateral_sc, 127, 100)
      , cv_gui_helper.make_new_trackbar_params( trackbar::sigma_space           , "bilateral sigma space (x100)", window::controller_1, current_finger_detector_conf_.pre_bilateral_ss, 127, 100)
      , cv_gui_helper.make_new_trackbar_params( trackbar::morphology_repeat     , "pre morphology repeat"       , window::controller_1, current_finger_detector_conf_.pre_morphology_n,  15)
      , cv_gui_helper.make_new_trackbar_params( trackbar::h_min                 , "H min   (x100)"              , window::controller_2, current_finger_detector_conf_.hsv_h_min, 720,   100)
      , cv_gui_helper.make_new_trackbar_params( trackbar::h_max                 , "H max   (x100)"              , window::controller_2, current_finger_detector_conf_.hsv_h_max, 720,   100)
      , cv_gui_helper.make_new_trackbar_params( trackbar::s_min                 , "S min (x10000)"              , window::controller_2, current_finger_detector_conf_.hsv_s_min,   1, 10000)
      , cv_gui_helper.make_new_trackbar_params( trackbar::s_max                 , "S max (x10000)"              , window::controller_2, current_finger_detector_conf_.hsv_s_max,   1, 10000)
      , cv_gui_helper.make_new_trackbar_params( trackbar::v_min                 , "V min         "              , window::controller_2, current_finger_detector_conf_.hsv_v_min, 255)
      , cv_gui_helper.make_new_trackbar_params( trackbar::v_max                 , "V max         "              , window::controller_2, current_finger_detector_conf_.hsv_v_max, 255)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_morphology       , "nail morphology repeat "     , window::controller_2, current_finger_detector_conf_.nail_morphology_n,  15)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_median_blur      , "nail median-blur kernel"     , window::controller_2, current_finger_detector_conf_.nail_median_blur_ksize,  35)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circles_dp        , "hough-circle dp"             , window::controller_2, current_finger_detector_conf_.circles_dp,  16)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circles_min_dist  , "hough-circle min dist"       , window::controller_2, current_finger_detector_conf_.circles_min_dist,  64)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circles_param_1   , "hough-circle param-1"        , window::controller_2, current_finger_detector_conf_.circles_param_1, 200)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circles_param_2   , "hough-circle param-2"        , window::controller_2, current_finger_detector_conf_.circles_param_2, 200)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circles_min_radius, "hough-circle min radius"     , window::controller_2, current_finger_detector_conf_.circles_min_radius,  48)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circles_max_radius, "hough-circle max radius"     , window::controller_2, current_finger_detector_conf_.circles_max_radius,  48)
      );
      
      DLOG(INFO) << "ctor end";
    }
    
    void gui_t::operator()(const input_t& input)
    {
      auto& cv_gui_helper = cv_gui_helper_t::instance();
      
      auto m_it = input.in_top.clone();
      auto m_if = input.in_front.clone();
      cv::Mat m_ot;
      cv::Mat m_of;
      
      cv::cvtColor(input.out_top  , m_ot, CV_GRAY2BGR);
      cv::cvtColor(input.out_front, m_of, CV_GRAY2BGR);
      
      cv_gui_helper.add_axes(m_it);
      cv_gui_helper.add_axes(m_if);
      cv_gui_helper.add_axes(m_ot);
      cv_gui_helper.add_axes(m_of);
      
      const auto add_circles = [](const finger_detector_t::circles_t& cs, cv::Mat& mi, cv::Mat& mo)
      {
        for(const auto v : cs)
        {
          const cv::Point center(int(std::round(v[0])), int(std::round(v[1])));
          const auto      radius = int(std::round(v[2]));
          for(auto frame: {mi, mo})
          {
            cv::circle(frame, center,      2, cv::Scalar(0,0xff,0), -1, 8, 0);
            cv::circle(frame, center, radius, cv::Scalar(0,0,0xff),  3, 8, 0);
          }
        }
      };
      
      add_circles(input.circles_top  , m_it, m_ot);
      add_circles(input.circles_front, m_if, m_of);
      
      cv_gui_helper.show(window::in_top    , m_it);
      cv_gui_helper.show(window::in_front  , m_if);
      cv_gui_helper.show(window::out_top   , m_ot);
      cv_gui_helper.show(window::out_front , m_of);
      
      current_finger_detector_conf_.pre_bilateral_d = cv_gui_helper.trackbar<double>(trackbar::diameter, window::controller_1);
      current_finger_detector_conf_.pre_bilateral_sc = cv_gui_helper.trackbar<double>(trackbar::sigma_color, window::controller_1);
      current_finger_detector_conf_.pre_bilateral_ss = cv_gui_helper.trackbar<double>(trackbar::sigma_space, window::controller_1);
      current_finger_detector_conf_.pre_morphology_n = cv_gui_helper.trackbar(trackbar::morphology_repeat, window::controller_1);
      current_finger_detector_conf_.hsv_h_min = cv_gui_helper.trackbar<double>(trackbar::h_min, window::controller_2);
      current_finger_detector_conf_.hsv_h_max = cv_gui_helper.trackbar<double>(trackbar::h_max, window::controller_2);
      current_finger_detector_conf_.hsv_s_min = cv_gui_helper.trackbar<double>(trackbar::s_min, window::controller_2);
      current_finger_detector_conf_.hsv_s_max = cv_gui_helper.trackbar<double>(trackbar::s_max, window::controller_2);
      current_finger_detector_conf_.hsv_v_min = cv_gui_helper.trackbar<double>(trackbar::v_min, window::controller_2);
      current_finger_detector_conf_.hsv_v_max = cv_gui_helper.trackbar<double>(trackbar::v_max, window::controller_2);
      current_finger_detector_conf_.nail_morphology_n = cv_gui_helper.trackbar(trackbar::nail_morphology, window::controller_2);
      current_finger_detector_conf_.nail_median_blur_ksize = cv_gui_helper.trackbar(trackbar::nail_median_blur, window::controller_2);
      current_finger_detector_conf_.circles_dp = cv_gui_helper.trackbar(trackbar::nail_circles_dp, window::controller_2);
      current_finger_detector_conf_.circles_min_dist = cv_gui_helper.trackbar(trackbar::nail_circles_min_dist, window::controller_2);
      current_finger_detector_conf_.circles_param_1 = cv_gui_helper.trackbar(trackbar::nail_circles_param_1, window::controller_2);
      current_finger_detector_conf_.circles_param_2 = cv_gui_helper.trackbar(trackbar::nail_circles_param_2, window::controller_2);
      current_finger_detector_conf_.circles_min_radius = cv_gui_helper.trackbar(trackbar::nail_circles_min_radius, window::controller_2);
      current_finger_detector_conf_.circles_max_radius = cv_gui_helper.trackbar(trackbar::nail_circles_max_radius, window::controller_2);
      
      if(prev_top_front_switch != cv_gui_helper.trackbar<int>(trackbar::top_front_switch, window::controller_1))
      {
        DLOG(INFO) << "top_front_switch changed";
        load_conf(bool(cv_gui_helper.trackbar<int>(trackbar::top_front_switch, window::controller_1) == 0));
      }
      
      if(cv_gui_helper.trackbar<int>(trackbar::save, window::controller_1))
      {
        save_conf(cv_gui_helper.trackbar<int>(trackbar::top_front_switch, window::controller_1) == 0);
        cv_gui_helper.trackbar(trackbar::save, window::controller_1, 0);
      }
      
      if(cv_gui_helper.trackbar<int>(trackbar::load, window::controller_1))
      {
        load_conf(cv_gui_helper.trackbar<int>(trackbar::top_front_switch, window::controller_1) == 0);
        cv_gui_helper.trackbar(trackbar::load, window::controller_1, 0);
      }
      
      prev_top_front_switch = cv_gui_helper.trackbar<int>(trackbar::top_front_switch, window::controller_1);
      
      //cv_gui_helper.present();
      cv_gui_helper.wait_key_not('\x1b', 1);
    }
    
    void gui_t::save_conf(bool is_top)
    {
      DLOG(INFO) << "save_conf: is_top(" << is_top << ")";
      
      if(is_top)
        conf_.finger_detector_top = current_finger_detector_conf_;
      else
        conf_.finger_detector_front = current_finger_detector_conf_;
      
      commandline_helper_t::save_conf(conf_);
    }
    
    void gui_t::load_conf(bool is_top)
    {
      DLOG(INFO) << "load_conf: is_top(" << is_top << ")";
      
      if(is_top)
        current_finger_detector_conf_ = conf_.finger_detector_top;
      else
        current_finger_detector_conf_ = conf_.finger_detector_front;
      
      auto& cv_gui_helper = cv_gui_helper_t::instance();
      cv_gui_helper.trackbar(trackbar::diameter, window::controller_1, current_finger_detector_conf_.pre_bilateral_d);
      cv_gui_helper.trackbar(trackbar::sigma_color, window::controller_1, current_finger_detector_conf_.pre_bilateral_sc);
      cv_gui_helper.trackbar(trackbar::sigma_space, window::controller_1, current_finger_detector_conf_.pre_bilateral_ss);
      cv_gui_helper.trackbar(trackbar::morphology_repeat, window::controller_1, current_finger_detector_conf_.pre_morphology_n);
      cv_gui_helper.trackbar(trackbar::h_min, window::controller_2, current_finger_detector_conf_.hsv_h_min);
      cv_gui_helper.trackbar(trackbar::h_max, window::controller_2, current_finger_detector_conf_.hsv_h_max);
      cv_gui_helper.trackbar(trackbar::s_min, window::controller_2, current_finger_detector_conf_.hsv_s_min);
      cv_gui_helper.trackbar(trackbar::s_max, window::controller_2, current_finger_detector_conf_.hsv_s_max);
      cv_gui_helper.trackbar(trackbar::v_min, window::controller_2, current_finger_detector_conf_.hsv_v_min);
      cv_gui_helper.trackbar(trackbar::v_max, window::controller_2, current_finger_detector_conf_.hsv_v_max);
      cv_gui_helper.trackbar(trackbar::nail_morphology, window::controller_2, current_finger_detector_conf_.nail_morphology_n);
      cv_gui_helper.trackbar(trackbar::nail_median_blur, window::controller_2, current_finger_detector_conf_.nail_median_blur_ksize);
      cv_gui_helper.trackbar(trackbar::nail_circles_dp, window::controller_2, current_finger_detector_conf_.circles_dp);
      cv_gui_helper.trackbar(trackbar::nail_circles_min_dist, window::controller_2, current_finger_detector_conf_.circles_min_dist);
      cv_gui_helper.trackbar(trackbar::nail_circles_param_1, window::controller_2, current_finger_detector_conf_.circles_param_1);
      cv_gui_helper.trackbar(trackbar::nail_circles_param_2, window::controller_2, current_finger_detector_conf_.circles_param_2);
      cv_gui_helper.trackbar(trackbar::nail_circles_min_radius, window::controller_2, current_finger_detector_conf_.circles_min_radius);
      cv_gui_helper.trackbar(trackbar::nail_circles_max_radius, window::controller_2, current_finger_detector_conf_.circles_max_radius);
      
    }
    
    const configuration_t::finger_detector_configuration_t& gui_t::current_finger_detector_conf() const
    { return current_finger_detector_conf_; }
    
    const bool gui_t::current_is_top() const
    {
      auto& cv_gui_helper = cv_gui_helper_t::instance();
      return cv_gui_helper.trackbar<int>(trackbar::top_front_switch, window::controller_1) == 0;
    }
    
  }
}