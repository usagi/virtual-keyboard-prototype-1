#include "finger-detector.hxx"

#include <algorithm>
#include <memory>
#include <limits>
#include <cassert>

#include "image_processor.hxx"

namespace arisin
{
  namespace etupirka
  {
    finger_detector_t::finger_detector_t(const configuration_t& conf, bool is_top)
    {
      DLOG(INFO) << "ctor";
      set(conf, is_top);
    }
    
    void finger_detector_t::set(const configuration_t& c, bool is_top)
    {
      DLOG(INFO) << "is_top: " << is_top;
      set
      ( is_top
          ? c.finger_detector_top
          : c.finger_detector_front
      );
    }
    
    void finger_detector_t::set(const configuration_t::finger_detector_configuration_t& c)
    {
      set_pre_bilateral(c.pre_bilateral_d, c.pre_bilateral_sc, c.pre_bilateral_ss);
      set_pre_morphology(c.pre_morphology_n);
      set_hsv(c.hsv_h_min, c.hsv_h_max, c.hsv_s_min, c.hsv_s_max, c.hsv_v_min, c.hsv_v_max);
      set_nail_morphology(c.nail_morphology_n);
      set_nail_median_blur(c.nail_median_blur_ksize);
      set_circles(c.circles_dp, c.circles_min_dist, c.circles_param_1, c.circles_param_2, c.circles_min_radius, c.circles_max_radius);
    }
    
    void finger_detector_t::set_pre_bilateral(double d, double sc, double ss)
    {
      pre_bilateral_d_ = d;
      pre_bilateral_sc_ = sc;
      pre_bilateral_ss_ = ss;
      DLOG(INFO) << "set_pre_bilateral d, sc, ss: " << d << ", " << sc << ", " << ss;
    }
    
    void finger_detector_t::set_pre_morphology(int n)
    {
      pre_morphology_n_ = n;
      DLOG(INFO) << "set_pre_morphology n: " << n;
    }
    
    void finger_detector_t::set_hsv(float hsv_h_min, float hsv_h_max, float hsv_s_min, float hsv_s_max, float hsv_v_min, float hsv_v_max)
    {
      hsv_h_min_ = hsv_h_min;
      hsv_h_max_ = hsv_h_max;
      hsv_s_min_ = hsv_s_min;
      hsv_s_max_ = hsv_s_max;
      hsv_v_min_ = hsv_v_min;
      hsv_v_max_ = hsv_v_max;
      DLOG(INFO) << "set_hsv h-min, h-max, s-min, s-max, v-min, v-max: " << hsv_h_min << ", " << hsv_h_max << ", " << hsv_s_min << ", " << hsv_s_max << ", " << hsv_v_min << ", " << hsv_v_max << ", ";
    }
    
    void finger_detector_t::set_nail_morphology(int n)
    {
      nail_morphology_n_ = n;
      DLOG(INFO) << "set_nail_morphology n: " << n;
    }
    
    void finger_detector_t::set_nail_median_blur(int ksize)
    {
      if(ksize % 2 == 0)
      {
        LOG(WARNING) << "ksize(" << ksize << ") is not even number, fix to " << ksize + 1;
        ++ksize;
      }
      
      nail_median_blur_ksize_ = ksize;
      DLOG(INFO) << "set_nail_median_blur ksize: " << ksize;
    }
    
    void finger_detector_t::set_circles(double dp, double min_dist, double param_1, double param_2, int min_radius, int max_radius)
    {
      if(dp < 1)
      {
        LOG(WARNING) << "dp(" << dp << ") cannot set less than 1, fix to 1";
        dp = 1;
      }
      
      if(min_dist < 1)
      {
        LOG(WARNING) << "min_dist(" << min_dist << ") cannot set less than 1, fix to 1";
        min_dist = 1;
      }
      
      circles_dp_ = dp;
      circles_min_dist_ = min_dist;
      circles_param_1_ = param_1;
      circles_param_2_ = param_2;
      circles_min_radius_ = min_radius;
      circles_max_radius_ = max_radius;
      DLOG(INFO) << "set_circles dp, min_dist, param_1, param_2, min_radius, max_radius: " << dp << ", " << min_dist << ", " << param_1 << ", " << param_2 << ", " << min_radius << ", " << max_radius;
    }
    
    const cv::Mat& finger_detector_t::effected_frame() const
    { return pre_nail_frame; }
    
    finger_detector_t::circles_t finger_detector_t::operator()(const cv::Mat& frame)
    {
      cv::Mat bilateral_frame;
      //bilateral_frame = frame;
      //cv::bilateralFilter(frame, bilateral_frame, pre_bilateral_d_, pre_bilateral_sc_, pre_bilateral_ss_);
      ::bilateralFilter_8u(frame, bilateral_frame, pre_bilateral_d_, pre_bilateral_sc_, pre_bilateral_ss_);
      
      cv::Mat morphology_frame;
      //morphology_frame = bilateral_frame;
      cv::morphologyEx(frame, morphology_frame, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), pre_morphology_n_);
      
      //cv::Mat hsv_filtered_single_channel_frame(frame.rows, frame.cols, CV_8UC1);
      //*
      const auto hsv_filtered_single_channel_frame = filter_hsv_from_BGR24_to_single_channel
      ( morphology_frame
      , hsv_h_min_, hsv_h_max_
      , hsv_s_min_, hsv_s_max_
      , hsv_v_min_, hsv_v_max_
      );
      //*/
      
      // morphology: single-channel
      cv::Mat single_channel_morphology_frame;
      //single_channel_morphology_frame = hsv_filtered_single_channel_frame;
      cv::morphologyEx(hsv_filtered_single_channel_frame, single_channel_morphology_frame, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), nail_morphology_n_);
        
      // median-blur: single-channel
      //pre_nail_frame = single_channel_morphology_frame;
      //cv::medianBlur(single_channel_morphology_frame, pre_nail_frame, nail_median_blur_ksize_);
      ::medianBlur(single_channel_morphology_frame, pre_nail_frame, nail_median_blur_ksize_);

      circles_t circles;
      //*
      {
        // circles detector
        cv::HoughCircles
        ( pre_nail_frame, circles, CV_HOUGH_GRADIENT
        , circles_dp_, circles_min_dist_
        , circles_param_1_, circles_param_2_
        , circles_min_radius_, circles_max_radius_
        );
        
        // circle filter
        boost::sort(circles, [](const cv::Vec3f& a, const cv::Vec3f& b){ return a[0] < b[0]; });
        const auto e = std::end(circles);
        auto t = std::begin(circles);
        for(auto i = std::begin(circles) + 1; i < e; ++i)
        {
          // i.x - i.r が t.x + t.r 以下にあるかチェック
          //   true : それは既存 t と比べて y 値が大きければ（より下にあれば） t と置換える
          //   false: それは次の t になる
          const auto i_left = (*i)[0] - (*i)[2];
          const auto t_right = (*t)[0] + (*t)[2];
          if( i_left <= t_right )
          {
            if( (*i)[1] > (*t)[1] )
              *t = *i;
          }
          else
            *++t = *i;
        }
        circles.resize(std::distance(std::begin(circles), t + 1));
      }
      //*/
#ifndef NDEBUG
      for(const auto& circle: circles)
        DLOG(INFO) << "circle x, y, r: " << circle[0] << ", " << circle[1] << ", " << circle[2];
#endif
      return circles;
    }
  }
}