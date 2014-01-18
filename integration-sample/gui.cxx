#include "gui.hxx"
#include "cv_gui_helper.hxx"

namespace arisin
{
  namespace etupirka
  {
    gui_t::gui_t(configuration_t& conf)
      : conf_(conf)
    {
      auto& cv_gui_helper = cv_gui_helper_t::instance();
      
      cv_gui_helper.new_windows
      ( cv_gui_helper.make_new_window_params( window::in_top      , "top-cam(input)"   , CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
      , cv_gui_helper.make_new_window_params( window::in_front    , "front-cam(input)" , CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
      , cv_gui_helper.make_new_window_params( window::out_top     , "top-cam(output)"  , CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
      , cv_gui_helper.make_new_window_params( window::out_front   , "front-cam(output)", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
      , cv_gui_helper.make_new_window_params( window::controller_1, "controller 1"     , CV_WINDOW_NORMAL | CV_WINDOW_FREERATIO | CV_GUI_EXPANDED)
      , cv_gui_helper.make_new_window_params( window::controller_2, "controller 2"     , CV_WINDOW_NORMAL | CV_WINDOW_FREERATIO | CV_GUI_EXPANDED)
      );
      
      cv_gui_helper.resize(window::in_top   , conf.camera_capture.width, conf.camera_capture.height);
      cv_gui_helper.resize(window::in_front , conf.camera_capture.width, conf.camera_capture.height);
      cv_gui_helper.resize(window::out_top  , conf.camera_capture.width, conf.camera_capture.height);
      cv_gui_helper.resize(window::out_front, conf.camera_capture.width, conf.camera_capture.height);
      
      cv_gui_helper.move(window::in_top      ,    0,   0);
      cv_gui_helper.move(window::in_front    ,    0, 480);
      cv_gui_helper.move(window::out_top     ,  640,   0);
      cv_gui_helper.move(window::out_front   ,  640, 480);
      cv_gui_helper.move(window::controller_1, 1280,   0);
      cv_gui_helper.move(window::controller_2, 1600,   0);
      
      cv_gui_helper.new_trackbars
      ( cv_gui_helper.make_new_trackbar_params( trackbar::top_front_switch      , "0:top/1:front switch"        , window::controller_1,   0,   1)
      , cv_gui_helper.make_new_trackbar_params( trackbar::save                  , "1:save"                      , window::controller_1,   0,   1)
      , cv_gui_helper.make_new_trackbar_params( trackbar::load                  , "1:load"                      , window::controller_1,   0,   1)
      , cv_gui_helper.make_new_trackbar_params( trackbar::diameter              , "bilateral diameter    (x100)", window::controller_1,  16, 127, 100)
      , cv_gui_helper.make_new_trackbar_params( trackbar::sigma_color           , "bilateral sigma color (x100)", window::controller_1,  72, 127, 100)
      , cv_gui_helper.make_new_trackbar_params( trackbar::sigma_space           , "bilateral sigma space (x100)", window::controller_1,  16, 127, 100)
      , cv_gui_helper.make_new_trackbar_params( trackbar::morphology_repeat     , "pre morphology repeat"       , window::controller_1,   5,  15)
      , cv_gui_helper.make_new_trackbar_params( trackbar::h_min                 , "H min   (x100)"              , window::controller_2, 315.87  , 720,   100)
      , cv_gui_helper.make_new_trackbar_params( trackbar::h_max                 , "H max   (x100)"              , window::controller_2, 356.36  , 720,   100)
      , cv_gui_helper.make_new_trackbar_params( trackbar::s_min                 , "S min (x10000)"              , window::controller_2,   0.2992,   1, 10000)
      , cv_gui_helper.make_new_trackbar_params( trackbar::s_max                 , "S max (x10000)"              , window::controller_2,   0.7049,   1, 10000)
      , cv_gui_helper.make_new_trackbar_params( trackbar::v_min                 , "V min         "              , window::controller_2, 120, 255)
      , cv_gui_helper.make_new_trackbar_params( trackbar::v_max                 , "V max         "              , window::controller_2, 255, 255)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_morphology       , "nail morphology repeat "     , window::controller_2,   5,  15)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_median_blur      , "nail median-blur kernel"     , window::controller_2,  13,  35)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circle_dp        , "hough-circle dp"             , window::controller_2,   1,  16)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circle_min_dist  , "hough-circle min dist"       , window::controller_2,   8,  64)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circle_param_1   , "hough-circle param-1"        , window::controller_2, 100, 200)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circle_param_2   , "hough-circle param-2"        , window::controller_2,   8, 200)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circle_min_radius, "hough-circle min radius"     , window::controller_2,   4,  48)
      , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circle_max_radius, "hough-circle max radius"     , window::controller_2,  12,  48)
      );
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
      
      //cv_gui_helper.present();
      cv_gui_helper.wait_key_not('\x1b', 1);
    }
  }
}