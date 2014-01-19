#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "configuration.hxx"
//#include "commandline_helper.hxx"
#include "finger-detector.hxx"

namespace arisin
{
  namespace etupirka
  {
    class gui_t final
    {
      enum class window
      { in_top, in_front
      , out_top, out_front
      , controller_1
      , controller_2
      };
      
      enum class trackbar
      { top_front_switch
      , save, load
      , diameter, sigma_color, sigma_space
      , morphology_repeat
      , h_min, h_max, s_min, s_max, v_min, v_max
      , nail_morphology, nail_median_blur
      , nail_circles_dp, nail_circles_min_dist
      , nail_circles_param_1, nail_circles_param_2
      , nail_circles_min_radius, nail_circles_max_radius
      };
      
      configuration_t& conf_;
      configuration_t::finger_detector_configuration_t current_finger_detector_conf_;
      int prev_top_front_switch;
      
      void save_conf(bool is_top = true);
      void load_conf(bool is_top = true);
      
    public:
      struct input_t
      {
        cv::Mat in_top , in_front ;
        cv::Mat out_top, out_front;
        finger_detector_t::circles_t circles_top, circles_front;
      };
      
      explicit gui_t(configuration_t& conf);
      void operator()(const input_t& input);
      
      const configuration_t::finger_detector_configuration_t& current_finger_detector_conf() const;
      const bool current_is_top() const;
    };
  }
}