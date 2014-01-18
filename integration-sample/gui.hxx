#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "configuration.hxx"
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
      , nail_circle_dp, nail_circle_min_dist
      , nail_circle_param_1, nail_circle_param_2
      , nail_circle_min_radius, nail_circle_max_radius
      };
      
      configuration_t& conf_;
      
    public:
      struct input_t
      {
        cv::Mat in_top , in_front ;
        cv::Mat out_top, out_front;
        finger_detector_t::circles_t circles_top, circles_front;
      };
      
      explicit gui_t(configuration_t& conf);
      void operator()(const input_t& input);
    };
  }
}