#pragma once

#include <array>
//#include <cassert>
#include <cstdint>
#include <chrono>
#include <functional>
//#include <fstream>
//#include <iostream>
//#include <iomanip>
#include <limits>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/range/algorithm.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui/highgui.hpp>

//#include "chrono_helper.hxx"
//#include "cv_mat_helper.hxx"
//#include "cv_mat_filter.hxx"
//#include "cv_gui_helper.hxx"
//#include "cv_video_helper.hxx"

#include "configuration.hxx"
#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    class finger_detector_t final
    {
      double pre_bilateral_d_;
      double pre_bilateral_sc_;
      double pre_bilateral_ss_;
      
      int pre_morphology_n_;
      
      float hsv_h_min_, hsv_h_max_, hsv_s_min_, hsv_s_max_, hsv_v_min_, hsv_v_max_;
      
      int nail_morphology_n_;
      int nail_median_blur_ksize_;
      
      double circles_dp_;
      double circles_min_dist_;
      double circles_param_1_;
      double circles_param_2_;
      int circles_min_radius_;
      int circles_max_radius_;
      
      cv::Mat pre_nail_frame;
      
    public:
      using circles_t = std::vector<cv::Vec3f>;
      
      finger_detector_t(const configuration_t& conf, bool is_top);
      
      void set(const configuration_t& c, bool is_top);
      void set(const configuration_t::finger_detector_configuration_t& c);
      
      void set_pre_bilateral(double d, double sc, double ss);
      void set_pre_morphology(int n);
      void set_hsv(float hsv_h_min, float hsv_h_max, float hsv_s_min, float hsv_s_max, float hsv_v_min, float hsv_v_max);
      void set_nail_morphology(int n);
      void set_nail_median_blur(int ksize);
      void set_circles(double dp, double min_dist, double param_1, double param_2, int min_radius, int max_radius);
      
      const cv::Mat& effected_frame() const;
      
      circles_t operator()(const cv::Mat& frame);
    };
  }
}